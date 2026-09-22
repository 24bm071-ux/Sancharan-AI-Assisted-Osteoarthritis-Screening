#include <HardwareSerial.h>
#include <math.h>

// ============================================================
// HLK-LD2402 + ESP32-C3 SUPER MINI
// GAIT RADAR - FAST FILTERED VERSION
//
// RAW DISTANCE
//      ↓
// RANGE CHECK
//      ↓
// OUTLIER REJECTION
//      ↓
// 3-SAMPLE MEDIAN
//      ↓
// LOW-PASS FILTER
//      ↓
// VELOCITY
//      ↓
// VELOCITY FILTER
//      ↓
// APPROACHING / LOW_RADIAL_MOTION / RECEDING
// ============================================================


// ============================================================
// PIN CONFIGURATION
// ============================================================

#define RADAR_RX  4
#define RADAR_TX  5
#define RADAR_IO  6

HardwareSerial RadarSerial(1);


// ============================================================
// DISTANCE LIMITS
// ============================================================

const float MIN_DISTANCE_CM = 30.0;
const float MAX_DISTANCE_CM = 600.0;


// ============================================================
// OUTLIER REJECTION
// ============================================================

// Maximum acceptable change between consecutive
// valid radar measurements.

const float MAX_JUMP_CM = 70.0;


// ============================================================
// MEDIAN FILTER
// ============================================================

// Reduced from 5 → 3 to reduce delay.

#define MEDIAN_SIZE 3

float medianBuffer[MEDIAN_SIZE];

int medianIndex = 0;
int medianCount = 0;


// ============================================================
// DISTANCE LOW-PASS FILTER
// ============================================================

// Higher alpha = faster response.
//
// Previous: 0.30
// Current : 0.60

const float DISTANCE_ALPHA = 0.60;


// ============================================================
// VELOCITY FILTER
// ============================================================

// Previous: 0.25
// Current : 0.40

const float SPEED_ALPHA = 0.40;


// ============================================================
// VELOCITY LIMIT
// ============================================================

const float MAX_SPEED_MPS = 3.0;


// ============================================================
// RADIAL MOTION THRESHOLD
// ============================================================

// Below ±0.05 m/s is considered
// LOW_RADIAL_MOTION.

const float RADIAL_MOTION_THRESHOLD = 0.05;


// ============================================================
// RADAR VARIABLES
// ============================================================

bool radarPresent = false;

float rawDistance = NAN;

float medianDistance = NAN;

float filteredDistance = NAN;

float previousFilteredDistance = NAN;


// ------------------------------------------------------------
// Velocity
// ------------------------------------------------------------

float rawVelocity = 0.0;

float filteredVelocity = 0.0;


// ------------------------------------------------------------
// Statistics
// ------------------------------------------------------------

float minimumDistance = 99999.0;

float maximumDistance = 0.0;

float peakApproachSpeed = 0.0;

float peakRecedeSpeed = 0.0;

float totalAbsoluteVelocity = 0.0;

unsigned long validSamples = 0;


// ============================================================
// TIMING
// ============================================================

unsigned long previousTime = 0;

unsigned long detectionStartTime = 0;

unsigned long detectionDuration = 0;

unsigned long detectionCount = 0;

unsigned long sampleInterval = 0;


// ============================================================
// UART BUFFER
// ============================================================

String radarLine = "";


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(1000);


  // ----------------------------------------------------------
  // LD2402 UART
  // ----------------------------------------------------------

  RadarSerial.begin(
    115200,
    SERIAL_8N1,
    RADAR_RX,
    RADAR_TX
  );


  // ----------------------------------------------------------
  // LD2402 IO
  // ----------------------------------------------------------

  pinMode(RADAR_IO, INPUT);


  // ----------------------------------------------------------
  // Startup information
  // ----------------------------------------------------------

  Serial.println();

  Serial.println(
    "============================================================"
  );

  Serial.println(
    "        HLK-LD2402 GAIT RADAR LOGGER"
  );

  Serial.println(
    "        ESP32-C3 SUPER MINI"
  );

  Serial.println(
    "============================================================"
  );

  Serial.println();

  Serial.println(
    "UART configuration:"
  );

  Serial.println(
    "Baud = 115200"
  );

  Serial.println(
    "Format = 8N1"
  );

  Serial.println();

  Serial.println(
    "Pin configuration:"
  );

  Serial.println(
    "LD2402 TX -> GPIO4"
  );

  Serial.println(
    "LD2402 RX -> GPIO5"
  );

  Serial.println(
    "LD2402 IO -> GPIO6"
  );

  Serial.println();

  Serial.print(
    "Valid distance: "
  );

  Serial.print(
    MIN_DISTANCE_CM
  );

  Serial.print(
    " - "
  );

  Serial.print(
    MAX_DISTANCE_CM
  );

  Serial.println(
    " cm"
  );

  Serial.print(
    "Median samples: "
  );

  Serial.println(
    MEDIAN_SIZE
  );

  Serial.print(
    "Distance alpha: "
  );

  Serial.println(
    DISTANCE_ALPHA
  );

  Serial.print(
    "Speed alpha: "
  );

  Serial.println(
    SPEED_ALPHA
  );

  Serial.println();

  Serial.println(
    "============================================================"
  );

  Serial.println();


  // ----------------------------------------------------------
  // CSV HEADER
  // ----------------------------------------------------------

  Serial.println(
    "time_ms,"
    "raw_cm,"
    "median_cm,"
    "filtered_cm,"
    "sample_interval_ms,"
    "raw_velocity_mps,"
    "filtered_velocity_mps,"
    "direction,"
    "presence"
  );
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop() {


  // ==========================================================
  // READ UART
  // ==========================================================

  while (RadarSerial.available()) {

    char c =
      RadarSerial.read();


    // --------------------------------------------------------
    // End of line
    // --------------------------------------------------------

    if (
      c == '\n' ||
      c == '\r'
    ) {

      if (
        radarLine.length() > 0
      ) {

        processRadarLine(
          radarLine
        );

        radarLine = "";
      }

    } else {

      // ------------------------------------------------------
      // Protect against oversized line
      // ------------------------------------------------------

      if (
        radarLine.length() < 80
      ) {

        radarLine += c;
      }
    }
  }


  // ==========================================================
  // UPDATE DETECTION DURATION
  // ==========================================================

  if (radarPresent) {

    detectionDuration =
      millis() -
      detectionStartTime;
  }
}


// ============================================================
// PROCESS RADAR LINE
// ============================================================

void processRadarLine(
  String line
) {

  line.trim();


  // ==========================================================
  // TARGET LOST
  // ==========================================================

  if (
    line.equalsIgnoreCase(
      "OFF"
    )
  ) {

    if (radarPresent) {

      radarPresent = false;

      detectionDuration =
        millis() -
        detectionStartTime;


      printDetectionSummary();
    }

    return;
  }


  // ==========================================================
  // DISTANCE MESSAGE
  // ==========================================================

  if (
    line.startsWith(
      "distance:"
    )
  ) {

    String value =
      line.substring(
        9
      );

    value.trim();


    float distance =
      value.toFloat();


    processDistance(
      distance
    );

    return;
  }
}


// ============================================================
// PROCESS DISTANCE
// ============================================================

void processDistance(
  float distance
) {

  rawDistance =
    distance;


  // ==========================================================
  // 1. RANGE VALIDATION
  // ==========================================================

  if (
    distance <
    MIN_DISTANCE_CM
    ||
    distance >
    MAX_DISTANCE_CM
  ) {

    Serial.print(
      "# RANGE REJECTED: "
    );

    Serial.print(
      distance,
      1
    );

    Serial.println(
      " cm"
    );

    return;
  }


  // ==========================================================
  // NEW DETECTION
  // ==========================================================

  if (
    !radarPresent
  ) {

    startNewDetection(
      distance
    );

    return;
  }


  // ==========================================================
  // 2. OUTLIER REJECTION
  // ==========================================================

  float difference =
    fabs(
      distance -
      filteredDistance
    );


  if (
    difference >
    MAX_JUMP_CM
  ) {

    Serial.print(
      "# OUTLIER REJECTED: "
    );

    Serial.print(
      distance,
      1
    );

    Serial.print(
      " cm | jump = "
    );

    Serial.print(
      difference,
      1
    );

    Serial.println(
      " cm"
    );

    return;
  }


  // ==========================================================
  // 3. MEDIAN FILTER
  // ==========================================================

  addToMedianBuffer(
    distance
  );


  medianDistance =
    calculateMedian();


  // ==========================================================
  // 4. LOW-PASS FILTER
  // ==========================================================

  filteredDistance =
    (
      DISTANCE_ALPHA *
      medianDistance
    )
    +
    (
      (1.0 -
       DISTANCE_ALPHA)
      *
      filteredDistance
    );


  // ==========================================================
  // UPDATE DISTANCE STATISTICS
  // ==========================================================

  if (
    filteredDistance <
    minimumDistance
  ) {

    minimumDistance =
      filteredDistance;
  }


  if (
    filteredDistance >
    maximumDistance
  ) {

    maximumDistance =
      filteredDistance;
  }


  // ==========================================================
  // 5. VELOCITY
  // ==========================================================

  unsigned long currentTime =
    millis();


  sampleInterval =
    currentTime -
    previousTime;


  if (
    sampleInterval > 0
  ) {

    float dt =
      sampleInterval /
      1000.0;


    float distanceChange =
      filteredDistance -
      previousFilteredDistance;


    float distanceChangeMeters =
      distanceChange /
      100.0;


    rawVelocity =
      distanceChangeMeters /
      dt;


    // --------------------------------------------------------
    // Reject unrealistic velocity
    // --------------------------------------------------------

    if (
      fabs(
        rawVelocity
      )
      >
      MAX_SPEED_MPS
    ) {

      rawVelocity =
        0.0;
    }


    // ========================================================
    // 6. VELOCITY LOW-PASS FILTER
    // ========================================================

    filteredVelocity =
      (
        SPEED_ALPHA *
        rawVelocity
      )
      +
      (
        (1.0 -
         SPEED_ALPHA)
        *
        filteredVelocity
      );


    // --------------------------------------------------------
    // Peak approaching speed
    // --------------------------------------------------------

    if (
      filteredVelocity < 0
    ) {

      float speed =
        fabs(
          filteredVelocity
        );


      if (
        speed >
        peakApproachSpeed
      ) {

        peakApproachSpeed =
          speed;
      }
    }


    // --------------------------------------------------------
    // Peak receding speed
    // --------------------------------------------------------

    if (
      filteredVelocity > 0
    ) {

      if (
        filteredVelocity >
        peakRecedeSpeed
      ) {

        peakRecedeSpeed =
          filteredVelocity;
      }
    }


    // --------------------------------------------------------
    // Average velocity calculation
    // --------------------------------------------------------

    totalAbsoluteVelocity +=
      fabs(
        filteredVelocity
      );
  }


  // ==========================================================
  // SAVE PREVIOUS VALUES
  // ==========================================================

  previousFilteredDistance =
    filteredDistance;

  previousTime =
    currentTime;


  validSamples++;


  // ==========================================================
  // PRINT
  // ==========================================================

  printMeasurement();
}


// ============================================================
// START NEW DETECTION
// ============================================================

void startNewDetection(
  float distance
) {

  radarPresent =
    true;


  detectionCount++;


  detectionStartTime =
    millis();


  detectionDuration =
    0;


  minimumDistance =
    distance;


  maximumDistance =
    distance;


  peakApproachSpeed =
    0.0;


  peakRecedeSpeed =
    0.0;


  totalAbsoluteVelocity =
    0.0;


  validSamples =
    1;


  // ----------------------------------------------------------
  // Reset median filter
  // ----------------------------------------------------------

  medianIndex =
    0;


  medianCount =
    MEDIAN_SIZE;


  for (
    int i = 0;
    i < MEDIAN_SIZE;
    i++
  ) {

    medianBuffer[i] =
      distance;
  }


  medianDistance =
    distance;


  filteredDistance =
    distance;


  previousFilteredDistance =
    distance;


  rawVelocity =
    0.0;


  filteredVelocity =
    0.0;


  previousTime =
    millis();


  sampleInterval =
    0;


  Serial.println();

  Serial.println(
    "# ===== NEW RADAR DETECTION ====="
  );


  printMeasurement();
}


// ============================================================
// ADD VALUE TO MEDIAN BUFFER
// ============================================================

void addToMedianBuffer(
  float value
) {

  medianBuffer[
    medianIndex
  ] =
    value;


  medianIndex++;


  if (
    medianIndex >=
    MEDIAN_SIZE
  ) {

    medianIndex =
      0;
  }


  if (
    medianCount <
    MEDIAN_SIZE
  ) {

    medianCount++;
  }
}


// ============================================================
// CALCULATE MEDIAN
// ============================================================

float calculateMedian() {

  float temp[
    MEDIAN_SIZE
  ];


  for (
    int i = 0;
    i < MEDIAN_SIZE;
    i++
  ) {

    temp[i] =
      medianBuffer[i];
  }


  // ----------------------------------------------------------
  // Sort
  // ----------------------------------------------------------

  for (
    int i = 0;
    i <
    MEDIAN_SIZE - 1;
    i++
  ) {

    for (
      int j = i + 1;
      j <
      MEDIAN_SIZE;
      j++
    ) {

      if (
        temp[j] <
        temp[i]
      ) {

        float swap =
          temp[i];

        temp[i] =
          temp[j];

        temp[j] =
          swap;
      }
    }
  }


  // ----------------------------------------------------------
  // Return middle value
  // ----------------------------------------------------------

  return temp[
    MEDIAN_SIZE / 2
  ];
}


// ============================================================
// DIRECTION
// ============================================================

String getDirection() {

  if (
    filteredVelocity <
    -RADIAL_MOTION_THRESHOLD
  ) {

    return "APPROACHING";
  }


  if (
    filteredVelocity >
    RADIAL_MOTION_THRESHOLD
  ) {

    return "RECEDING";
  }


  return "LOW_RADIAL_MOTION";
}


// ============================================================
// PRINT MEASUREMENT
// ============================================================

void printMeasurement() {

  Serial.print(
    millis()
  );

  Serial.print(",");


  // Raw distance

  Serial.print(
    rawDistance,
    1
  );

  Serial.print(",");


  // Median distance

  Serial.print(
    medianDistance,
    1
  );

  Serial.print(",");


  // Filtered distance

  Serial.print(
    filteredDistance,
    1
  );

  Serial.print(",");


  // Sample interval

  Serial.print(
    sampleInterval
  );

  Serial.print(",");


  // Raw velocity

  Serial.print(
    rawVelocity,
    3
  );

  Serial.print(",");


  // Filtered velocity

  Serial.print(
    filteredVelocity,
    3
  );

  Serial.print(",");


  // Direction

  Serial.print(
    getDirection()
  );

  Serial.print(",");


  // Presence

  Serial.println(
    radarPresent ?
    1 :
    0
  );
}


// ============================================================
// DETECTION SUMMARY
// ============================================================

void printDetectionSummary() {

  Serial.println();

  Serial.println(
    "============================================================"
  );

  Serial.println(
    "                 RADAR SUMMARY"
  );

  Serial.println(
    "============================================================"
  );


  Serial.print(
    "Detection number       : "
  );

  Serial.println(
    detectionCount
  );


  Serial.print(
    "Detection duration     : "
  );

  Serial.print(
    detectionDuration
  );

  Serial.println(
    " ms"
  );


  Serial.print(
    "Minimum distance       : "
  );

  Serial.print(
    minimumDistance,
    1
  );

  Serial.println(
    " cm"
  );


  Serial.print(
    "Maximum distance       : "
  );

  Serial.print(
    maximumDistance,
    1
  );

  Serial.println(
    " cm"
  );


  Serial.print(
    "Peak approach speed    : "
  );

  Serial.print(
    peakApproachSpeed,
    3
  );

  Serial.println(
    " m/s"
  );


  Serial.print(
    "Peak receding speed    : "
  );

  Serial.print(
    peakRecedeSpeed,
    3
  );

  Serial.println(
    " m/s"
  );


  Serial.print(
    "Valid samples          : "
  );

  Serial.println(
    validSamples
  );


  if (
    validSamples > 0
  ) {

    float averageVelocity =
      totalAbsoluteVelocity /
      validSamples;


    Serial.print(
      "Average |velocity|     : "
    );

    Serial.print(
      averageVelocity,
      3
    );

    Serial.println(
      " m/s"
    );
  }


  Serial.println(
    "============================================================"
  );

  Serial.println();
}
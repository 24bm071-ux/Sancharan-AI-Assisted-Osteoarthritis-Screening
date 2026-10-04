#include <HardwareSerial.h>
#include <math.h>

// ============================================================
// HLK-LD2420 + ESP32-C3 SUPER MINI
// HUMAN MOTION / GAIT RADAR
//
// LD2420 UART format:
//
// ON
// Range 105
// Range 103
// Range 101
// ...
// OFF
//
// CSV OUTPUT:
//
// time_ms,
// raw_cm,
// median_cm,
// filtered_cm,
// sample_interval_ms,
// raw_velocity_mps,
// filtered_velocity_mps,
// direction,
// presence
// ============================================================


// ============================================================
// PIN CONFIGURATION
// ============================================================

// HLK-LD2420:
// OT1 -> UART TX from radar
// RX  -> UART RX to radar
// OT2 -> Presence output

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

// Maximum acceptable distance jump
// between two consecutive measurements.

const float MAX_JUMP_CM = 70.0;


// ============================================================
// MEDIAN FILTER
// ============================================================

#define MEDIAN_SIZE 3

float medianBuffer[MEDIAN_SIZE];

int medianIndex = 0;
int medianCount = 0;


// ============================================================
// DISTANCE LOW-PASS FILTER
// ============================================================

// Higher alpha = faster response

const float DISTANCE_ALPHA = 0.60;


// ============================================================
// VELOCITY LOW-PASS FILTER
// ============================================================

const float SPEED_ALPHA = 0.40;


// ============================================================
// VELOCITY LIMIT
// ============================================================

const float MAX_SPEED_MPS = 3.0;


// ============================================================
// RADIAL MOTION THRESHOLD
// ============================================================

// Below ±0.05 m/s = LOW_RADIAL_MOTION

const float RADIAL_MOTION_THRESHOLD = 0.05;


// ============================================================
// RADAR VARIABLES
// ============================================================

bool radarPresent = false;

float rawDistance = NAN;

float medianDistance = NAN;

float filteredDistance = NAN;

float previousFilteredDistance = NAN;


// ============================================================
// VELOCITY VARIABLES
// ============================================================

float rawVelocity = 0.0;

float filteredVelocity = 0.0;


// ============================================================
// STATISTICS
// ============================================================

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
// FUNCTION DECLARATIONS
// ============================================================

void processRadarLine(String line);

void processDistance(float distance);

void startNewDetection(float distance);

void addToMedianBuffer(float value);

float calculateMedian();

String getDirection();

void printMeasurement();

void printDetectionSummary();


// ============================================================
// SETUP
// ============================================================

void setup()
{
  // ----------------------------------------------------------
  // USB SERIAL
  // ----------------------------------------------------------

  Serial.begin(115200);

  delay(1000);


  // ----------------------------------------------------------
  // HLK-LD2420 UART
  // ----------------------------------------------------------

  RadarSerial.begin(
    115200,
    SERIAL_8N1,
    RADAR_RX,
    RADAR_TX
  );


  // ----------------------------------------------------------
  // OT2 PRESENCE OUTPUT
  // ----------------------------------------------------------

  pinMode(
    RADAR_IO,
    INPUT
  );


  // ----------------------------------------------------------
  // STARTUP MESSAGE
  // ----------------------------------------------------------

  Serial.println();

  Serial.println(
    "============================================================"
  );

  Serial.println(
    "              HLK-LD2420 RADAR LOGGER"
  );

  Serial.println(
    "              ESP32-C3 SUPER MINI"
  );

  Serial.println(
    "============================================================"
  );

  Serial.println();


  // ----------------------------------------------------------
  // UART INFORMATION
  // ----------------------------------------------------------

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


  // ----------------------------------------------------------
  // PIN INFORMATION
  // ----------------------------------------------------------

  Serial.println(
    "Pin configuration:"
  );

  Serial.println(
    "LD2420 OT1 (TX) -> ESP32 GPIO4"
  );

  Serial.println(
    "LD2420 RX       -> ESP32 GPIO5"
  );

  Serial.println(
    "LD2420 OT2      -> ESP32 GPIO6"
  );

  Serial.println();


  // ----------------------------------------------------------
  // FILTER INFORMATION
  // ----------------------------------------------------------

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


  // ==========================================================
  // CSV HEADER
  // ==========================================================

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

void loop()
{

  // ==========================================================
  // READ LD2420 UART
  // ==========================================================

  while (RadarSerial.available())
  {

    char c =
      RadarSerial.read();


    // --------------------------------------------------------
    // END OF LINE
    // --------------------------------------------------------

    if (
      c == '\n' ||
      c == '\r'
    )
    {

      if (
        radarLine.length() > 0
      )
      {

        processRadarLine(
          radarLine
        );

        radarLine = "";
      }
    }

    else
    {

      // ------------------------------------------------------
      // PROTECT UART BUFFER
      // ------------------------------------------------------

      if (
        radarLine.length() < 80
      )
      {

        radarLine += c;
      }
    }
  }


  // ==========================================================
  // UPDATE DETECTION DURATION
  // ==========================================================

  if (radarPresent)
  {

    detectionDuration =
      millis() -
      detectionStartTime;
  }
}


// ============================================================
// PROCESS RADAR UART LINE
// ============================================================

void processRadarLine(
  String line
)
{

  line.trim();


  // Ignore empty lines

  if (
    line.length() == 0
  )
  {
    return;
  }


  // ==========================================================
  // PERSON DETECTED
  // ==========================================================

  if (
    line.equalsIgnoreCase(
      "ON"
    )
  )
  {

    radarPresent = true;

    // IMPORTANT:
    //
    // Do NOT initialize filteredDistance here.
    //
    // LD2420 sends ON before Range.
    //
    // We wait for:
    //
    // Range xxx

    return;
  }


  // ==========================================================
  // PERSON LOST
  // ==========================================================

  if (
    line.equalsIgnoreCase(
      "OFF"
    )
  )
  {

    if (
      radarPresent
    )
    {

      radarPresent = false;


      detectionDuration =
        millis() -
        detectionStartTime;


      printDetectionSummary();
    }

    return;
  }


  // ==========================================================
  // RANGE MESSAGE
  //
  // Example:
  //
  // Range 105
  // Range 97
  // Range 120
  // ==========================================================

  if (
    line.startsWith(
      "Range "
    )
    ||
    line.startsWith(
      "range "
    )
  )
  {

    String value =
      line.substring(
        6
      );


    value.trim();


    float distance =
      value.toFloat();


    if (
      distance > 0
    )
    {

      processDistance(
        distance
      );
    }

    return;
  }
}


// ============================================================
// PROCESS DISTANCE
// ============================================================

void processDistance(
  float distance
)
{

  // ==========================================================
  // SAVE RAW DISTANCE
  // ==========================================================

  rawDistance =
    distance;


  // ==========================================================
  // RANGE VALIDATION
  // ==========================================================

  if (
    distance < MIN_DISTANCE_CM ||
    distance > MAX_DISTANCE_CM
  )
  {

    return;
  }


  // ==========================================================
  // FIRST VALID DISTANCE
  // ==========================================================

  // IMPORTANT FIX:
  //
  // LD2420 sequence is:
  //
  // ON
  // Range 41
  //
  // radarPresent is already TRUE because of ON.
  //
  // Therefore we also check:
  //
  // isnan(filteredDistance)
  //
  // This prevents filteredDistance from becoming NAN.

  if (
    !radarPresent ||
    isnan(filteredDistance)
  )
  {

    startNewDetection(
      distance
    );

    return;
  }


  // ==========================================================
  // OUTLIER REJECTION
  // ==========================================================

  float difference =
    fabs(
      distance -
      filteredDistance
    );


  if (
    difference >
    MAX_JUMP_CM
  )
  {

    return;
  }


  // ==========================================================
  // MEDIAN FILTER
  // ==========================================================

  addToMedianBuffer(
    distance
  );


  medianDistance =
    calculateMedian();


  // ==========================================================
  // LOW-PASS DISTANCE FILTER
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
  // DISTANCE STATISTICS
  // ==========================================================

  if (
    filteredDistance <
    minimumDistance
  )
  {

    minimumDistance =
      filteredDistance;
  }


  if (
    filteredDistance >
    maximumDistance
  )
  {

    maximumDistance =
      filteredDistance;
  }


  // ==========================================================
  // CURRENT TIME
  // ==========================================================

  unsigned long currentTime =
    millis();


  sampleInterval =
    currentTime -
    previousTime;


  // ==========================================================
  // VELOCITY
  // ==========================================================

  if (
    sampleInterval > 0 &&
    !isnan(previousFilteredDistance)
  )
  {

    float dt =
      sampleInterval /
      1000.0;


    // --------------------------------------------------------
    // Distance change
    // --------------------------------------------------------

    float distanceChange =
      filteredDistance -
      previousFilteredDistance;


    // --------------------------------------------------------
    // Convert cm → m
    // --------------------------------------------------------

    float distanceChangeMeters =
      distanceChange /
      100.0;


    // --------------------------------------------------------
    // Velocity
    // --------------------------------------------------------

    rawVelocity =
      distanceChangeMeters /
      dt;


    // ========================================================
    // VELOCITY LIMIT
    // ========================================================

    if (
      fabs(
        rawVelocity
      )
      >
      MAX_SPEED_MPS
    )
    {

      rawVelocity =
        0.0;
    }


    // ========================================================
    // VELOCITY LOW-PASS FILTER
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


    // ========================================================
    // PEAK APPROACHING SPEED
    // ========================================================

    if (
      filteredVelocity < 0
    )
    {

      float speed =
        fabs(
          filteredVelocity
        );


      if (
        speed >
        peakApproachSpeed
      )
      {

        peakApproachSpeed =
          speed;
      }
    }


    // ========================================================
    // PEAK RECEDING SPEED
    // ========================================================

    if (
      filteredVelocity > 0
    )
    {

      if (
        filteredVelocity >
        peakRecedeSpeed
      )
      {

        peakRecedeSpeed =
          filteredVelocity;
      }
    }


    // ========================================================
    // TOTAL ABSOLUTE VELOCITY
    // ========================================================

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
  // PRINT RESULT
  // ==========================================================

  printMeasurement();
}


// ============================================================
// START NEW DETECTION
// ============================================================

void startNewDetection(
  float distance
)
{

  // ==========================================================
  // PRESENCE
  // ==========================================================

  radarPresent =
    true;


  // ==========================================================
  // DETECTION COUNT
  // ==========================================================

  detectionCount++;


  // ==========================================================
  // START TIME
  // ==========================================================

  detectionStartTime =
    millis();


  detectionDuration =
    0;


  // ==========================================================
  // DISTANCE STATISTICS
  // ==========================================================

  minimumDistance =
    distance;


  maximumDistance =
    distance;


  // ==========================================================
  // VELOCITY STATISTICS
  // ==========================================================

  peakApproachSpeed =
    0.0;


  peakRecedeSpeed =
    0.0;


  totalAbsoluteVelocity =
    0.0;


  validSamples =
    1;


  // ==========================================================
  // RESET MEDIAN FILTER
  // ==========================================================

  medianIndex =
    0;


  medianCount =
    MEDIAN_SIZE;


  for (
    int i = 0;
    i < MEDIAN_SIZE;
    i++
  )
  {

    medianBuffer[i] =
      distance;
  }


  // ==========================================================
  // INITIALIZE FILTER VALUES
  // ==========================================================

  medianDistance =
    distance;


  filteredDistance =
    distance;


  previousFilteredDistance =
    distance;


  // ==========================================================
  // INITIALIZE VELOCITY
  // ==========================================================

  rawVelocity =
    0.0;


  filteredVelocity =
    0.0;


  // ==========================================================
  // INITIALIZE TIME
  // ==========================================================

  previousTime =
    millis();


  sampleInterval =
    0;


  // ==========================================================
  // PRINT FIRST MEASUREMENT
  // ==========================================================

  printMeasurement();
}


// ============================================================
// ADD VALUE TO MEDIAN BUFFER
// ============================================================

void addToMedianBuffer(
  float value
)
{

  medianBuffer[
    medianIndex
  ] =
    value;


  medianIndex++;


  if (
    medianIndex >=
    MEDIAN_SIZE
  )
  {

    medianIndex =
      0;
  }


  if (
    medianCount <
    MEDIAN_SIZE
  )
  {

    medianCount++;
  }
}


// ============================================================
// CALCULATE MEDIAN
// ============================================================

float calculateMedian()
{

  float temp[
    MEDIAN_SIZE
  ];


  // ==========================================================
  // COPY BUFFER
  // ==========================================================

  for (
    int i = 0;
    i < MEDIAN_SIZE;
    i++
  )
  {

    temp[i] =
      medianBuffer[i];
  }


  // ==========================================================
  // SORT
  // ==========================================================

  for (
    int i = 0;
    i < MEDIAN_SIZE - 1;
    i++
  )
  {

    for (
      int j = i + 1;
      j < MEDIAN_SIZE;
      j++
    )
    {

      if (
        temp[j] <
        temp[i]
      )
      {

        float swap =
          temp[i];


        temp[i] =
          temp[j];


        temp[j] =
          swap;
      }
    }
  }


  // ==========================================================
  // RETURN MIDDLE VALUE
  // ==========================================================

  return temp[
    MEDIAN_SIZE / 2
  ];
}


// ============================================================
// DIRECTION
// ============================================================

String getDirection()
{

  // ----------------------------------------------------------
  // APPROACHING
  // ----------------------------------------------------------

  if (
    filteredVelocity <
    -RADIAL_MOTION_THRESHOLD
  )
  {

    return "APPROACHING";
  }


  // ----------------------------------------------------------
  // RECEDING
  // ----------------------------------------------------------

  if (
    filteredVelocity >
    RADIAL_MOTION_THRESHOLD
  )
  {

    return "RECEDING";
  }


  // ----------------------------------------------------------
  // LOW RADIAL MOTION
  // ----------------------------------------------------------

  return "LOW_RADIAL_MOTION";
}


// ============================================================
// PRINT MEASUREMENT
// ============================================================

void printMeasurement()
{

  // ==========================================================
  // TIME
  // ==========================================================

  Serial.print(
    millis()
  );

  Serial.print(",");


  // ==========================================================
  // RAW DISTANCE
  // ==========================================================

  Serial.print(
    rawDistance,
    1
  );

  Serial.print(",");


  // ==========================================================
  // MEDIAN DISTANCE
  // ==========================================================

  Serial.print(
    medianDistance,
    1
  );

  Serial.print(",");


  // ==========================================================
  // FILTERED DISTANCE
  // ==========================================================

  Serial.print(
    filteredDistance,
    1
  );

  Serial.print(",");


  // ==========================================================
  // SAMPLE INTERVAL
  // ==========================================================

  Serial.print(
    sampleInterval
  );

  Serial.print(",");


  // ==========================================================
  // RAW VELOCITY
  // ==========================================================

  Serial.print(
    rawVelocity,
    3
  );

  Serial.print(",");


  // ==========================================================
  // FILTERED VELOCITY
  // ==========================================================

  Serial.print(
    filteredVelocity,
    3
  );

  Serial.print(",");


  // ==========================================================
  // DIRECTION
  // ==========================================================

  Serial.print(
    getDirection()
  );

  Serial.print(",");


  // ==========================================================
  // PRESENCE
  // ==========================================================

  Serial.println(
    radarPresent
      ? 1
      : 0
  );
}


// ============================================================
// DETECTION SUMMARY
// ============================================================

void printDetectionSummary()
{

  Serial.println();

  Serial.println(
    "============================================================"
  );

  Serial.println(
    "                    RADAR SUMMARY"
  );

  Serial.println(
    "============================================================"
  );


  // ==========================================================
  // DETECTION NUMBER
  // ==========================================================

  Serial.print(
    "Detection number       : "
  );

  Serial.println(
    detectionCount
  );


  // ==========================================================
  // DURATION
  // ==========================================================

  Serial.print(
    "Detection duration     : "
  );

  Serial.print(
    detectionDuration
  );

  Serial.println(
    " ms"
  );


  // ==========================================================
  // MINIMUM DISTANCE
  // ==========================================================

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


  // ==========================================================
  // MAXIMUM DISTANCE
  // ==========================================================

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


  // ==========================================================
  // PEAK APPROACH SPEED
  // ==========================================================

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


  // ==========================================================
  // PEAK RECEDING SPEED
  // ==========================================================

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


  // ==========================================================
  // VALID SAMPLES
  // ==========================================================

  Serial.print(
    "Valid samples          : "
  );

  Serial.println(
    validSamples
  );


  // ==========================================================
  // AVERAGE VELOCITY
  // ==========================================================

  if (
    validSamples > 0
  )
  {

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

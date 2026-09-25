# Sancharan — AI-Assisted Multimodal Osteoarthritis Screening

Sancharan is an AI-assisted, non-invasive multimodal sensing system designed to investigate osteoarthritis (OA)-related functional risk markers through analysis of knee movement, gait, muscle activity, acoustic signals, and contactless motion.

## Project Overview

Osteoarthritis can affect movement, mobility, muscle function, and gait. In low-resource and underserved healthcare settings, access to specialized assessment can be limited.

Sancharan explores a portable and low-cost approach that combines multiple sensing modalities to capture complementary information about lower-limb function.

The system currently integrates four sensing modalities:

- **IMU** — measures thigh and shank movement, acceleration, and angular velocity for knee kinematics and gait analysis.
- **Acoustic sensing** — captures acoustic signal variations during knee movement for signal analysis.
- **EMG** — measures electrical muscle activity to study muscle activation and neuromuscular patterns.
- **Radar** — provides contactless distance and motion measurements.

The collected signals are processed to extract meaningful features, which can then be investigated individually and through multimodal AI fusion.

## System Architecture

```text
Patient
   ↓
Functional Movement Tests
   ↓
┌──────────┬───────────┬──────────┬──────────┐
│   IMU    │ Acoustic  │   EMG    │  Radar   │
└──────────┴───────────┴──────────┴──────────┘
        ↓
Signal Acquisition
        ↓
Signal Processing
        ↓
Feature Extraction
        ↓
Multimodal Feature Fusion
        ↓
AI / Machine Learning
        ↓
OA-Related Functional Risk Profile
        ↓
Further Clinical Evaluation

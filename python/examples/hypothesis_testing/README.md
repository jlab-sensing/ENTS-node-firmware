# Hypothesis Testing Framework

This directory contains dedicated, reproducible statistical pipelines for testing formal scientific hypotheses regarding soil bioelectrochemical dynamics, trajectory similarity, and environmental invariance.

All markdown reports and README files feature embedded figures using relative paths configured for seamless online rendering on GitHub.

---

## Directory Structure

```
hypothesis_testing/
+-- README.md                          <- Master index and methodology overview
+-- soil_similarity/                   <- Hypothesis 1: Soil Trajectory Similarity
|   +-- README.md                      <- Documentation, data ranges, and rendered plots
|   +-- soil_similarity_test.py        <- Executable analysis runner
|   +-- soil_similarity_config.yaml    <- Configuration parameters & cell mappings
|   \-- output/                        <- Generated plots and evaluation report (hypothesis_evaluation_report.md)
+-- texture_cycles/                    <- Hypothesis 2: Texture & Wetting/Drying Cycles
|   +-- README.md                      <- Documentation, data ranges, and rendered plots
|   +-- texture_cycle_test.py          <- Executable analysis runner
|   +-- texture_cycle_config.yaml      <- Configuration parameters & soil compositions
|   \-- output/                        <- Generated plots and evaluation report (texture_cycle_report.md)
+-- ambient_site_variation/            <- Hypothesis 3: Cross-Site Ambient Environmental Invariance
|   +-- README.md                      <- Documentation, data ranges, and rendered plots
|   +-- ambient_site_variation_test.py <- Executable analysis runner
|   +-- ambient_site_variation_config.yaml <- Multi-site configuration & monitored units
|   \-- output/                        <- Generated plots and evaluation report (ambient_site_variation_report.md)
\-- pca_parameter_relationship/        <- Multivariate PCA with Guaranteed Sample Independence
    +-- README.md                      <- Documentation, methodology, and rendered plots
    +-- pca_parameter_relationship_test.py <- Executable analysis runner
    +-- pca_parameter_relationship_config.yaml <- Analysis configuration & feature definitions
    \-- output/                        <- Generated plots and evaluation report (pca_parameter_relationship_report.md)
```

---

## Data Ranges & Data Augmentation

### Data Ranges Analyzed
- **Hypothesis 1 (Soil Similarity)**: September 15, 2025 - March 9, 2026 (175 days total: 70-day startup/incubation phase, 105-day stabilized open-circuit phase).
- **Hypothesis 2 (Texture & Cycles)**: March 12, 2026 - June 15, 2026 (95 days total continuous closed-circuit operation).
- **Hypothesis 3 (Cross-Site Invariance)**: September 15, 2025 - March 9, 2026 (175 days concurrent multi-site monitoring across independent geographic deployments).
- **PCA Parameter Relationship**: Both closed-circuit power matrix (March 12 - June 15, 2026) and multi-site environmental coupling (September 15, 2025 - March 9, 2026) with 14- to 21-day decorrelation thinning for sample independence.
- **Sampling Resolution**: Hourly resampled regular intervals (`resample="hour"`).

### Data Augmentation, Quality Screening & Preprocessing
1. **Temporal Regularization**: Raw observations resampled to regular 1-hour time intervals.
2. **Multi-Channel Alignment & Cleaning**: Synchronized multi-channel time-series (potentials, power, soil moisture, temperature, conductivity, and ambient covariates), dropping incomplete rows for balanced model estimation.
3. **Instrumentation Fidelity Benchmarking**: Validated measurement nodes against co-located laboratory-grade Source Measure Unit (SMU) references (achieving $r = 0.9887$ for moisture and $r = 0.9924$ for soil temperature), confirming field sensor fidelity.
4. **Current Signal Anomaly Screening**: Integrated automated physical consistency filters ($P \ge 0$, polarity verification) based on multi-site data screening to eliminate current channel inversion and saturation dropouts.
5. **Metric Centering**: Centered temporal metrics (`time_c`) to prevent multicollinearity in longitudinal interaction modeling.
6. **Dynamic Derivative Extraction & Smoothing**: Derived numerical moisture rates of change ($d\text{VWC}/dt$), smoothed via a 6-hour rolling median filter (`d_vwc_smooth`).
7. **Hydrological State & Event Segmentation**: Tagged dynamic wetting, drying, and quiescent states, segmenting contiguous events to extract event-level power swings ($\Delta P$) and dynamic moisture sensitivities ($\Delta P / \Delta \text{VWC}$).
8. **Saturation Stratum Segmentation**: Stratified observations into discrete moisture brackets (`Low <30%`, `Moderate 30-35%`, `High 35-40%`, `Saturated >40%`) to isolate fixed saturation regimes from transient moisture swings.
9. **Continuous Textural Covariate Linking**: Mapped laboratory-measured continuous sand, silt, and clay fractions to observations for regression modeling.

---

## The Three-Tier Statistical Methodology

1. **Tier 1: Global Statistical Modeling (LMM)**: Repeated-Measures Linear Mixed-Effects Models controlling for cell-level random intercepts and environmental covariates (`raw_vwc`, `temp`, `amb_temp`).
2. **Tier 2: Targeted Hypothesis Criteria**: Problem-specific tests such as Two One-Sided Tests (TOST) for similarity and contrast invariance, and event-based segmentation for wetting/drying cycles.
3. **Tier 3: Physical & Mechanistic Synthesis**: Translating statistical variance into soil physics, aeration barriers, thermal damping, and electrochemistry.

---

## Quick Start

```bash
# Hypothesis 1: Test whether amended red soil matches native black soil
python python/examples/hypothesis_testing/soil_similarity/soil_similarity_test.py

# Hypothesis 2: Test how soil texture and wetting/drying cycles impact power
python python/examples/hypothesis_testing/texture_cycles/texture_cycle_test.py

# Hypothesis 3: Test whether ambient parameters explain between-site variations
python python/examples/hypothesis_testing/ambient_site_variation/ambient_site_variation_test.py
```

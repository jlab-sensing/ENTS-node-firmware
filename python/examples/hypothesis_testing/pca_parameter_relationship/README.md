# Principal Component Analysis (PCA) of Bioelectrochemical & Environmental Parameters

This module implements a rigorous Principal Component Analysis (PCA) pipeline designed to quantify the orthogonal multivariate relationships between measured bioelectrochemical, hydrological, soil matrix, and ambient meteorological parameters while strictly enforcing and verifying **sample independence**.

## Problem Formulation & Sample Independence

In continuous environmental and sensor monitoring, raw time series violate the fundamental independent and identically distributed ($i.i.d.$) assumption of PCA due to strong serial autocorrelation ($\rho_1 > 0.95$) and 24-hour diurnal cycling. Naive PCA on autocorrelated data inflates effective sample size, biases covariance estimates, and compresses variance into trivial diurnal oscillation artifacts.

To guarantee true sample independence, this pipeline:
1. Profiles empirical autocorrelation functions (ACF) to determine the decorrelation timescale ($\tau_{\text{decorr}}$).
2. Decimates daily aggregated summaries at intervals $\ge \tau_{\text{decorr}}$ ($14\text{ to }21\text{ days}$).
3. Formally tests within-cell serial independence using the Ljung-Box test ($p > 0.05$) and Durbin-Watson statistic ($DW \approx 1.5\text{--}2.2$).
4. Retains statistically verified principal components using Horn's Parallel Analysis with Monte Carlo feature permutation and the Kaiser-Guttman criterion ($\lambda \ge 1.0$).

## Measured Parameters Evaluated

### 1. Bioelectrochemical Power & Matrix Architecture (Closed-Circuit Regime)
- **Power Output ($P$, $\mu\text{W}$)**
- **Operating Voltage ($V$, $\text{mV}$)**
- **Operating Current ($I$, $\mu\text{A}$)**
- **Internal Resistance ($R_{\text{cell}} = V / I$, $\text{k}\Omega$)**
- **Volumetric Water Content ($\text{VWC}$, $\%$ )**
- **Soil Temperature ($T_{\text{soil}}$, $^\circ\text{C}$)**
- **Soil Bulk Electrical Conductivity ($\text{EC}$, $\mu\text{S/cm}$)**
- **Sand Fraction ($f_{\text{sand}}$, $\%$ )**
- **Clay Fraction ($f_{\text{clay}}$, $\%$ )**

### 2. Multi-Site Environmental & Hydrological Parameter Space
- **Open-Circuit Cell Potential ($V$, $\text{mV}$)**
- **Volumetric Water Content ($\text{VWC}$, $\%$ )**
- **Soil Temperature ($T_{\text{soil}}$, $^\circ\text{C}$)**
- **Soil Bulk Electrical Conductivity ($\text{EC}$, $\mu\text{S/cm}$)**
- **Ambient Air Temperature ($T_{\text{amb}}$, $^\circ\text{C}$)**
- **Ambient Relative Humidity ($\text{RH}$, $\%$ )**
- **Barometric Pressure ($P_{\text{atm}}$, $\text{hPa}$)**
- **Vapor Pressure Deficit ($\text{VPD}$, $\text{kPa}$)**
- **Soil-Air Thermal Differential ($\Delta T = T_{\text{soil}} - T_{\text{amb}}$, $^\circ\text{C}$)**

## Execution

To execute the pipeline and regenerate all figures and markdown reports:

```bash
TEMPORARILY_DISABLE_PROTOBUF_VERSION_CHECK=true .venv/bin/python python/examples/hypothesis_testing/pca_parameter_relationship/pca_parameter_relationship_test.py
```

## Generated Artifacts

- **Report**: [pca_parameter_relationship_report.md](output/pca_parameter_relationship_report.md)
- **Diagnostic Figures**:
  - `autocorrelation_decorrelation_timeline.png`: ACF decay curves and empirical decorrelation thresholds.
  - `pca_scree_parallel_analysis.png`: Scree plots with Horn's Parallel Analysis 95th percentile cutoffs.
  - `pca_power_matrix_biplot.png`: Biplot and variable correlation circle for bioelectrochemical parameters.
  - `pca_environmental_biplot.png`: Biplot and variable correlation circle for multi-site environmental parameters.
  - `naive_vs_independent_pca_comparison.png`: Direct comparison between naive autocorrelated PCA vs. decorrelated independent PCA.

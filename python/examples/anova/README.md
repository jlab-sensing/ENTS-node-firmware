# ANOVA for MOTE

## Usage

```
python -m venv .venv
source .venv/bin/activate
pip install -e [PATH to ENTS]
./anova.py
```


## Description

`anova.py` - Runs simple test on a single cell.

From dirtviz data is collected every hour. Can get individual data points.


## Resources


### Anova
- [scipy anova](https://docs.scipy.org/doc/scipy/reference/generated/scipy.stats.f_oneway.html)
- [statsmodels anova](https://www.statsmodels.org/stable/anova.html)
- [statsmodels function](https://www.statsmodels.org/stable/generated/statsmodels.stats.anova.anova_lm.html#statsmodels.stats.anova.anova_lm)
- [statsmodels example](https://www.statsmodels.org/stable/examples/notebooks/generated/interactions_anova.html)

### Mixedlm

- [statsmodels mixedlm](https://www.statsmodels.org/stable/mixed_linear.html)



## ANOVA Information

### Assumptions

1. The samples are independent.
1. Each sample is from a normally distributed population.
1. The population standard deviations of the groups are all equal. This property is known as homoscedasticity.

Otherwise look at *Kruskal-Wallis H-test* or *Alexander-Govern test*.

### Results

- *F-statistic:* Measures how the groups means differ relative to noise.
- *p-value:* Probability of observing an F-value if the null hypothesis is true.

For SMFCs:
- *F-statistic:* Ratio of explained variance (due to a soil factor, e.g., moisture, pH, organic carbon class) to unexplained variance (within-condition noise).
- *p-value:* Probability of observing an F at least this large if the soil factor has no real effect on SMFC power.
    - Small -> reject "no-soil effect"
    - Large -> insufficient evidence that soil variable matter

Example Results:
- F = 9.3, p = 0.002: SMFC power differs significantly across soil conditions
- F = 1.1, p = 0.34: Differences in power are consistent with random variability.


*Eta squared:* Fraction of total variance explained by a factor.
*Partial eta squared:* Fraction of remaining variances explained by a factor


### Multiple Sensors

For multiple sensors there is a `yyyyyyy` model that handles multiple sensors.

```
from statsmodels.formula.api import mixedlm

model = mixedlm(
    "voltage ~ vwc + ec + amb_temp + amb_hum + amb_press",
    df,
    groups=df["sensor_id"]
).fit()
```


## Outputs

### OLS

```
Checking multicollinearity:


Rules of thumb for variation inflation factor:
VIF > 5		Moderate
VIF > 10	Severe
VIF = inf	Linear dependence
  variable           VIF
0    const  11212.892422
1       ec      4.282459
2  raw_vwc      6.838088
3     temp      1.065113
4     sand     13.159674


ANOVA Results:

                sum_sq       df            F        PR(>F)    eta_sq  part_eta_sq
ec        1.347579e+07      1.0   216.430810  1.079715e-48  0.010552     0.012528
raw_vwc   1.684776e+08      1.0  2705.869167  0.000000e+00  0.131925     0.136903
temp      9.050469e+06      1.0   145.356963  2.451734e-33  0.007087     0.008449
sand      2.390975e+07      1.0   384.007583  1.418962e-84  0.018722     0.022015
Residual  1.062157e+09  17059.0          NaN           NaN  0.831714     0.500000
```

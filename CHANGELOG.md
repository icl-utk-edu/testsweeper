2025.05.28 (ABI 2.0.0)
  - Added print_stats for --repeat
  - Added "skipped" status
  - Removed deprecated enum functions

2024.05.31 (ABI 1.0.0)
  - Added shared library ABI version
  - Updated enum parameters to have `to_string`, `from_string`;
    deprecate `<enum>2str`, `str2<enum>`

2023.11.05
  - Improve set_default to reset value
  - Add half precision

2023.08.25
  - Use yyyy.mm.dd version scheme, instead of yyyy.mm.release
  - Fixes for Intel clang compiler
  - Bug fixes and more robust testing

2023.06.00
  - Add metric (k, M, G), binary (Ki, Mi, Gi), and exponent (1e3) notation
    for integers
  - Update string parsing

2023.01.00
  - Move repo to GitHub: https://github.com/icl-utk-edu/testsweeper/
  - Adjustable column widths
  - Use python3

2021.04.00
  - Add ParamComplex
  - Fix bug in printing header

2020.09.00
  - CMake improvements to use as sub-project

2020.06.00
  - Convert repo to git

2020.03.00
  - Initial release

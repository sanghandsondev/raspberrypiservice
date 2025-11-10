# #!/bin/bash

# # *********************************************************
# # STATIC ANALYSIS: using 'cppcheck'
# # command: ./do_static_analysis.sh
# # output: static_report/cppcheck_static_analysis_report.log
# # *********************************************************

# TASK_NAME="do_static_analysis"
# REPORT_DIR=static_report
# IGNORE_DIR="-i .git/ -i .vscode/ -i build/ -i docs/ -i external/ -i $REPORT_DIR/"

# echo -e "\n [$TASK_NAME] start to install 'cppcheck'\n"
# sudo apt update -y && sudo apt install cppcheck -y
# mkdir -p $REPORT_DIR/
# echo -e "\n [$TASK_NAME] analysing...\n"
# cppcheck -f --enable=all -j 4 --std=c++20 $IGNORE_DIR . \
#   >$REPORT_DIR/cppcheck_static_analysis_report.log 2>&1
# echo -e "\n [$TASK_NAME] complete static analysis\n"

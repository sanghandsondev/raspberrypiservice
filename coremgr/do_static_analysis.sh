#!/bin/bash

# *********************************************************
# STATIC ANALYSIS: using 'cppcheck'
# command: ./do_static_analysis.sh
# output: static_report/cppcheck_static_analysis_report.log
# *********************************************************

TASK_NAME="do_static_analysis"
REPORT_DIR=static_report
IGNORE_DIR="-i .git/ -i .vscode/ -i build/ -i docs/ -i external/ -i $REPORT_DIR/"

echo -e "\n [$TASK_NAME] start to install 'cppcheck'\n"
sudo apt update -y && sudo apt install cppcheck -y
mkdir -p $REPORT_DIR/
echo -e "\n [$TASK_NAME] analysing...\n"

# Run cppcheck and generate XML output
mkdir -p $REPORT_DIR/html_report
cppcheck -f --enable=all --suppress=information --std=c++17 -j 4 $IGNORE_DIR . \
  --xml-version=2 2>$REPORT_DIR/cppcheck_report.xml

# Convert XML to HTML report
cppcheck-htmlreport --file=$REPORT_DIR/cppcheck_report.xml \
  --report-dir=$REPORT_DIR/html_report \
  --source-dir=.

# Generate text log
cppcheck -f --enable=all --suppress=information --std=c++17 -j 4 $IGNORE_DIR . \
  >$REPORT_DIR/cppcheck_static_analysis_report.log 2>&1

echo -e "\n [$TASK_NAME] complete static analysis\n"
echo -e "Reports generated at:"
echo -e "- Text report: $REPORT_DIR/cppcheck_static_analysis_report.log"
echo -e "- HTML report: $REPORT_DIR/html_report/index.html\n"
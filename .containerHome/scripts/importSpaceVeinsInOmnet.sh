#!/bin/bash

set -e

echo "Importing space_Veins in the OMNeT++ IDE..."

# Does currently not work. From the error log: URI: file:/workspace/space_veins is not valid in the workspace!
xvfb-run /opt/omnetpp/bin/omnetpp -data $HOME/omnetpp.workspace -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -printErrorMarkers -importAll $WORKSPACE >> report.log
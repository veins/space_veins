import csv
import numpy as np
from skyfield.api import load, EarthSatellite, wgs84
from skyfield.sgp4lib import theta_GMST1982
from skyfield.constants import AU_KM
import argparse
import datetime

parser = argparse.ArgumentParser(description='Compute satellite WGS84 positions.')
parser.add_argument('tleFile', metavar='TLE_FILE', help='File which stores the two line element sets.')
parser.add_argument('sdate', metavar='START_DATE', help='UTC start date, format: [yyyy-mm-dd:hh-mm-ss].')
parser.add_argument('duration', metavar='DURATION', type=float, help='Duration of the simulation in seconds.')
parser.add_argument('leoMobilityUpdateInterval', metavar='INTERVAL', type=float, help='Time interval between two periodic LEO satellite mobility updates in seconds.')

def compute_and_save_trajectory(tleFile, ts, t):

    # Load the satellites
    satellites = load.tle_file(tleFile)

    # Initialize the satellite object
    satellite = satellites[0]
    print("Computing positions...")

    # Calculate the geocentric positions and convert to WGS84 geographic coordinates
    geocentric = satellite.at(t)

    teme = satellite._position_and_velocity_TEME_km(t)
    rITRF, vITRF, error = satellite.ITRF_position_velocity_error(t)
    rITRF *= AU_KM
    # theta, theta_dot = theta_GMST1982(t.whole, t.ut1_fraction)
    # print(t.whole)
    # print(t.ut1_fraction)
    # print(t.dut1)
    # print(theta)
    # print(theta_dot)

    subpoint = wgs84.subpoint(geocentric)
    latitudes = subpoint.latitude.degrees
    longitudes = subpoint.longitude.degrees
    elevations = subpoint.elevation.m

    # Generate formatted UTC time strings for the CSV (including microseconds)
    utc_times = t.utc_strftime('%Y-%m-%d %H:%M:%S.%f')

    # Define the output CSV filename
    csv_filename = "satellite_positions.csv"

    print(f"Writing data to {csv_filename}...")
    
    with open(csv_filename, mode='w', newline='') as csv_file:
        writer = csv.writer(csv_file)

        writer.writerow(["Time_UTC", "Offset_Seconds", "teme_x_km", "teme_y_km", "teme_z_km", "itrf_x_km", "itrf_y_km", "itrf_z_km", "Latitude_deg", "Longitude_deg", "Elevation_m"])

        for i in range(len(seconds_offset)):
            writer.writerow([
                utc_times[i],
                f"{seconds_offset[i]:.2f}",
                f"{teme[0][0][i]:.9f}",
                f"{teme[0][1][i]:.9f}",
                f"{teme[0][2][i]:.9f}",
                f"{rITRF[0][i]:.9f}",
                f"{rITRF[1][i]:.9f}",
                f"{rITRF[2][i]:.9f}",
                f"{latitudes[i]:.9f}",
                f"{longitudes[i]:.9f}",
                f"{elevations[i]:.9f}",
            ])

    print(f"Success! {len(seconds_offset)} rows written to {csv_filename}.")

if __name__ == "__main__":
    args = parser.parse_args()
    seconds_offset = np.arange(0, args.duration + args.leoMobilityUpdateInterval, args.leoMobilityUpdateInterval)
    # start
    # date
    start = args.sdate.split(':')
    start_date = start[0]
    start_date = start_date.split("-")
    # time
    start_time = start[1]
    start_time = start_time.split("-")
    # datetime
    datetimeStartDate = datetime.datetime(int(start_date[0]), int(start_date[1]), int(start_date[2]), int(start_time[0]), int(start_time[1]), int(start_time[2]))
    # Load the standard Skyfield timescale
    ts = load.timescale()
    t0 = ts.utc(int(start_date[0]), int(start_date[1]), int(start_date[2]), int(start_time[0]), int(start_time[1]), seconds_offset)
    compute_and_save_trajectory(args.tleFile, ts, t0)

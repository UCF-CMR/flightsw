import os, xlrd, pickle
import numpy as np

def parse_mission_report(reportfile):
    rhead = []
    rdata = {}
    reportpickle = reportfile.replace(".xlsx", ".xlsx.pickle")
    if os.path.exists(reportpickle) and os.path.isfile(reportpickle):
        print("Unpickling mission report %s ..." % reportpickle)
        rhead, rdata = pickle.load(open(reportpickle, 'rb'))
    elif os.path.exists(reportfile) and os.path.isfile(reportfile):
        print("Parsing mission report %s ..." % reportfile)
        workbook = xlrd.open_workbook(reportfile)
        worksheet = workbook.sheet_by_index(2)
        for nrows in range(1, worksheet.nrows):
            if worksheet.cell_value(nrows, 2) <= 0.: break
        for col in range(worksheet.ncols):
            rhead.append(worksheet.cell_value(0, col))
            rdata[rhead[col]] = []
            for row in range(1, nrows):
                relem = worksheet.cell_value(row, col)
                if isinstance(relem, str):
                    relem = relem.strip()
                rdata[rhead[col]].append(relem)
            rdata[rhead[col]] = np.array(rdata[rhead[col]])
        print("Pickling mission report %s ..." % reportpickle)
        with open(reportpickle, 'wb') as fh:
            pickle.dump((rhead, rdata), fh)
    else:
        reportfile = None
        print("[WARN] Mission report file %s not found!" % reportfile)
    print()
    print(rhead)
    print()
    return rhead, rdata

def parse_flight_profile(profile):
    phead = {}
    pdata = []
    if os.path.exists(profile) and os.path.isfile(profile):
        print("Flight profile detected! Parsing %s ..." % profile)
        with open(profile, 'r') as fh:
            for line in fh:
                if not line.startswith('#'):
                    line = line.strip().split(':')
                    if len(line) == 2:
                        phead[line[0]] = eval(line[1])
                    elif len(line) == 1:
                        dataline = line[0].split(',')
                        pdata.append([float(dataline[0])]+dataline[1:])

        if 'OFFSET' in phead:
            for data in pdata:
                data[0] += phead['OFFSET']

        print()

    return phead, pdata

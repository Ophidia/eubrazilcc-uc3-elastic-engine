/*
    EUBrazilCC UC3 Elastic Engine
    Copyright 2014-2015 EUBrazilCC (EU‐Brazil Cloud Connect)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __QUERY_H
#define __QUERY_H

//Macros to define MetaDB queries
#define QUERYLEN 1024

#define QUERY_METADB_GET_PENDING_EXPERIMENTS "\
SELECT idexperiment, '1' AS number \
FROM experiment \
WHERE status like 'pending'  AND available = 1 AND stored = 0 \
ORDER BY idexperiment ASC;"

#define QUERY_METADB_GET_PENDING_EXPERIMENTS_CLEARING "\
SELECT idexperiment, submissiondate \
FROM experiment \
WHERE stored = 1 AND status like 'done' AND available = 1 \
ORDER BY idexperiment ASC;"

#define QUERY_METADB_GET_OLD_EXPERIMENTS "\
SELECT idexperiment, stored, submissiondate FROM experiment \
WHERE stored = 0 AND (status like 'done' OR status like 'failed' OR status like 'nodata') AND available = 1 AND enddate < DATE_SUB(CURDATE(),INTERVAL 5 DAY) OR deleted = 1 AND (stored = 0 OR stored = 2 OR stored = 3) AND (status like 'done' OR status like 'failed' OR status like 'nodata') AND available = 1 \
ORDER BY idexperiment ASC \
FOR UPDATE;"
#define QUERY_METADB_UPDATE_OLD_EXPERIMENTS "UPDATE experiment SET available = 0 WHERE stored = 0 AND (status like 'done' OR status like 'failed' OR status like 'nodata') AND available = 1 AND enddate < DATE_SUB(CURDATE(),INTERVAL 5 DAY) OR deleted = 1 AND (stored = 0 OR stored = 2 OR stored = 3) AND (status like 'done' OR status like 'failed' OR status like 'nodata') AND available = 1;"

#define QUERY_METADB_GET_EXPERIMENT_FILES "\
SELECT DISTINCT name, extension  \
FROM experimentoutput \
WHERE idexperiment = %lld \
ORDER BY idexperiment ASC;"

#define QUERY_METADB_GET_EXPERIMENT_TYPE "\
SELECT workflow.code  \
FROM experiment INNER JOIN workflow ON experiment.idworkflow = workflow.idworkflow \
WHERE experiment.idexperiment = %lld;"

#define QUERY_METADB_GET_EXP_SEBALINTERANNUAL_INFO "\
SELECT workflow.code, workflow.workflowfile, sebalinterannual.xmin, sebalinterannual.xmax, sebalinterannual.ymin, sebalinterannual.ymax, sebalinterannual.timemin, sebalinterannual.timemax, dataset.filename, dataset.folder, dataset.coordinate, satellitedata.timerange, satellitedata.timefrequency, satellitedata.basetime, satellitedata.units, satellitedata.calendar, satellitedata.variablecode, satellitedata.variablename, satellitedata.pdasid \
FROM experiment INNER JOIN workflow ON experiment.idworkflow =  workflow.idworkflow \
INNER JOIN sebalinterannual ON sebalinterannual.idexperiment = experiment.idexperiment \
INNER JOIN dataset ON dataset.iddataset = sebalinterannual.iddataset \
INNER JOIN satellitedata ON satellitedata.iddataset = dataset.iddataset \
WHERE experiment.idexperiment = %lld;"

#define QUERY_METADB_GET_EXP_CLIMATESEBAL_INFO_COMMON "\
SELECT workflow.code, workflow.workflowfile, climatesebal.xmin, climatesebal.xmax, climatesebal.ymin, climatesebal.ymax, climatesebal.timemin, climatesebal.timemax \
FROM experiment INNER JOIN workflow ON experiment.idworkflow =  workflow.idworkflow \
INNER JOIN climatesebal ON climatesebal.idexperiment = experiment.idexperiment \
WHERE experiment.idexperiment = %lld;"
#define QUERY_METADB_GET_EXP_CLIMATESEBAL_INFO_SATELLITE "\
SELECT dataset.filename, dataset.folder, dataset.coordinate, satellitedata.timerange, satellitedata.timefrequency, satellitedata.basetime, satellitedata.units, satellitedata.calendar, satellitedata.variablecode, satellitedata.variablename, satellitedata.pdasid \
FROM climatesebal INNER JOIN climatesebaldataset ON climatesebal.idexperiment = climatesebaldataset.idexperiment \
INNER JOIN dataset ON dataset.iddataset = climatesebaldataset.iddataset \
INNER JOIN satellitedata ON satellitedata.iddataset = dataset.iddataset \
WHERE climatesebal.idexperiment = %lld;"
#define QUERY_METADB_GET_EXP_CLIMATESEBAL_INFO_CLIMATE "\
SELECT dataset.filename, dataset.folder, dataset.coordinate, climatedata.timerange, climatedata.timefrequency, climatedata.basetime, climatedata.units, climatedata.calendar, climatedata.variablecode, climatedata.variablename \
FROM climatesebal INNER JOIN climatesebaldataset ON climatesebal.idexperiment = climatesebaldataset.idexperiment \
INNER JOIN dataset ON dataset.iddataset = climatesebaldataset.iddataset \
INNER JOIN climatedata ON climatedata.iddataset = dataset.iddataset \
WHERE climatesebal.idexperiment = %lld;"

#define QUERY_METADB_GET_EXP_ENM_COMMON_INFO "\
SELECT DISTINCT workflow.code, workflow.workflowfile, dataset.filename \
FROM experiment INNER JOIN workflow ON experiment.idworkflow =  workflow.idworkflow \
INNER JOIN enm ON experiment.idexperiment =  enm.idexperiment \
INNER JOIN speciesdata ON speciesdata.idoccurrence = enm.idoccurrence \
INNER JOIN dataset ON dataset.iddataset = speciesdata.iddataset \
WHERE experiment.idexperiment = %lld;"
#define QUERY_METADB_GET_EXP_ENM_POINTS_INFO "\
SELECT speciesdata.x, speciesdata.y \
FROM speciesdata INNER JOIN enm ON speciesdata.idoccurrence =  enm.idoccurrence \
WHERE enm.idexperiment = %lld;"

#define QUERY_METADB_GET_EXP_RELHEIGHT_INFO "\
SELECT workflow.code, workflow.workflowfile, dataset.filename, dataset.folder, dataset.coordinate \
FROM experiment INNER JOIN workflow ON experiment.idworkflow =  workflow.idworkflow \
INNER JOIN relheight ON relheight.idexperiment = experiment.idexperiment \
INNER JOIN dataset ON dataset.iddataset = relheight.iddataset \
WHERE experiment.idexperiment = %lld;"

#define QUERY_METADB_GET_EXP_LIDARINTERCOMPARISON_INFO "\
SELECT workflow.code, workflow.workflowfile, dataset.filename, dataset.folder, dataset.coordinate, dtm, dsm, chm, rh, agb, fc, aspect, sa, pd \
FROM experiment INNER JOIN workflow ON experiment.idworkflow =  workflow.idworkflow \
INNER JOIN lidarintercomparison ON lidarintercomparison.idexperiment = experiment.idexperiment \
INNER JOIN dataset ON dataset.iddataset = lidarintercomparison.iddataset \
WHERE experiment.idexperiment = %lld;"

#define QUERY_METADB_GET_EXP_MODELINTERCOMPARISON_INFO_COMMON "\
SELECT workflow.code, workflow.workflowfile, modelintercomparison.xmin, modelintercomparison.xmax, modelintercomparison.ymin, modelintercomparison.ymax, modelintercomparison.timemin, modelintercomparison.timemax, tnn, tnx, txn, txx \
FROM experiment INNER JOIN workflow ON experiment.idworkflow =  workflow.idworkflow \
INNER JOIN modelintercomparison ON modelintercomparison.idexperiment = experiment.idexperiment \
WHERE experiment.idexperiment = %lld;"
#define QUERY_METADB_GET_EXP_MODELINTERCOMPARISON_INFO_DATA "\
SELECT DISTINCT dataset.folder, dataset.coordinate, climatedata.timerange, climatedata.timefrequency, climatedata.basetime, climatedata.units, climatedata.calendar \
FROM  dataset INNER JOIN climatedata ON climatedata.iddataset = dataset.iddataset \
WHERE climatedata.variablecode like 'tasmax' or climatedata.variablecode like 'tasmin';"
#define QUERY_METADB_GET_EXP_MODELINTERCOMPARISON_INFO_MODEL "SELECT DISTINCT modelcode FROM  climatedata WHERE climatedata.variablecode like 'tasmax' or climatedata.variablecode like 'tasmin';"
#define QUERY_METADB_GET_EXP_MODELINTERCOMPARISON_INFO_SCENARIO "SELECT DISTINCT scenariocode FROM  climatedata WHERE climatedata.variablecode like 'tasmax' or climatedata.variablecode like 'tasmin';"

#define QUERY_METADB_ADD_EXPERIMENT_SEBALINTERANNUAL "INSERT INTO experimentoutput(idexperiment, name, extension, type, title) VALUES(%lld, 'out1.csv', 'csv','download','SEBAL interannual analysis'),(%lld, 'out1_grid.json', 'json','grid',''),(%lld, 'out1_chart.json', 'json','chart',''),(%lld, 'out2.csv', 'csv','download','SEBAL interannual statistics'),(%lld, 'out2_grid.json', 'json','grid',''),(%lld, 'out2_chart.json', 'json','chart','');"

#define QUERY_METADB_ADD_EXPERIMENT_CLIMATESEBAL "INSERT INTO experimentoutput(idexperiment, name, extension, type, title) VALUES(%lld, 'out1.csv', 'csv','download','Climate SEBAL intercomparison'),(%lld, 'out1_grid.json', 'json','grid',''),(%lld, 'out1_chart.json', 'json','chart',''),(%lld, 'out2.csv', 'csv','',''),(%lld, 'out2_grid.json', 'json','grid',''),(%lld, 'out2_chart.json', 'json','chart','');"

#define QUERY_METADB_ADD_EXPERIMENT_RELHEIGHT "INSERT INTO experimentoutput(idexperiment, name, extension, type, title) VALUES(%lld, 'out1_rh25.tif', 'tif','download','Relative Height 25'),(%lld, 'out1_rh50.tif', 'tif','download','Relative Height 50'),(%lld, 'out1_rh66.tif', 'tif','download','Relative Height 66'),(%lld, 'out1_rh75.tif', 'tif','download','Relative Height 75'),(%lld, 'out1_rh95.tif', 'tif','download','Relative Height 95'),(%lld, 'out1_grid.json', 'json','grid',''),(%lld, 'out1_histograms.png', 'png','image',''),(%lld, 'out1_image.png', 'png','image','');"

#define QUERY_METADB_ADD_EXPERIMENT_ENM "INSERT INTO experimentoutput(idexperiment, name, extension, type, title) VALUES(%lld, 'out1.img', 'img','download','Present Scenario'),(%lld, 'out2.img', 'img','download','Optimistic 2070 Scenario'),(%lld, 'out3.img', 'img','download','Pessimistic 2070 Scenario'),(%lld, 'out1.png', 'png','image','Present Scenario'),(%lld, 'out2.png', 'png','image','Optimistic 2070 Scenario'),(%lld, 'out3.png', 'png','image','Pessimistic 2070 Scenario');"

#define QUERY_METADB_ADD_EXPERIMENT_LIDARINTERCOMPARISON_FIXED "INSERT INTO experimentoutput(idexperiment, name, extension, type, title) VALUES(%lld, 'out1_grid.json', 'json','grid',''),(%lld, 'out1_scatter.png', 'png','image',''),(%lld, 'out1_image.png', 'png','image','');"
#define QUERY_METADB_ADD_EXPERIMENT_LIDARINTERCOMPARISON_DYNAMIC "INSERT INTO experimentoutput(idexperiment, name, extension, type, title) VALUES(%lld, 'out1_%s.tif', 'tif','download','%s');"

#define QUERY_METADB_ADD_EXPERIMENT_MODELINTERCOMPARISON_FIXED "INSERT INTO experimentoutput(idexperiment, name, extension, type, title) VALUES(%lld, 'out1.csv', 'csv','download','Climate Model Intercomparison'),(%lld, 'out2.csv', 'csv','download','Climate Model Intercomparison Statistics'),(%lld, 'out1_grid.json', 'json','grid',''),(%lld, 'out2_grid.json', 'json','grid','');"
#define QUERY_METADB_ADD_EXPERIMENT_MODELINTERCOMPARISON_DYNAMIC "INSERT INTO experimentoutput(idexperiment, name, extension, type, title) VALUES(%lld, 'out1_%d_chart.json', 'json','chart','');"

#define QUERY_METADB_SET_ASSIGNED_EXPERIMENT "UPDATE experiment SET status = 'assigned' WHERE idexperiment = %lld;"

#define QUERY_METADB_SET_RUNNING_EXPERIMENT "UPDATE experiment SET status = 'running', pdasworkflowstring= \"%s\"  WHERE idexperiment = %lld;"

#define QUERY_METADB_SET_COMPLETED_EXPERIMENT "UPDATE experiment SET status = 'done', pdasworkflowid = '%s', enddate = NOW()  WHERE idexperiment = %lld;"

#define QUERY_METADB_SET_FAILED_EXPERIMENT "UPDATE experiment SET status = 'failed', enddate = NOW() WHERE idexperiment = %lld;"

#define QUERY_METADB_SET_COMPLETED_EXPERIMENT_CLEARING "UPDATE experiment SET stored = 2 WHERE idexperiment = %lld;"

#define QUERY_METADB_SET_FAILED_EXPERIMENT_CLEARING "UPDATE experiment SET stored = 3 WHERE idexperiment = %lld;"


#define QUERY_METADB_ADD_CLUSTER_HISTORY "INSERT INTO clusterhistory(idthread, jobnum, status, address) VALUES(%d, %d, %d, '%s');"

#define QUERY_METADB_DELETE_OLD_STATUS "DELETE FROM clusterhistory WHERE timestamp < (NOW() - INTERVAL 5 MINUTE);"

#define QUERY_METADB_REMOVE_CLUSTER_CURRENT "DELETE FROM clustercurrent WHERE idthread = %d;"

#define QUERY_METADB_ADD_CLUSTER_CURRENT "INSERT INTO clustercurrent(idthread, jobnum, status, address) VALUES(%d, %d, %d, '%s');"

#define QUERY_METADB_DELETE_CLUSTERHIST "DELETE FROM clusterhistory;"

#define QUERY_METADB_DELETE_CLUSTERCURR "DELETE FROM clustercurrent;"

#define QUERY_METADB_UPDATE_TRANSIENT_EXEC_EXPERIMENT "UPDATE experiment SET status = 'failed', enddate = NOW() WHERE (status like 'pending' OR status like 'running' OR status like 'assigned') AND stored = 0 AND available = 1;"

#define QUERY_METADB_UPDATE_TRANSIENT_CH_EXPERIMENT "UPDATE experiment SET stored = 3 WHERE (status like 'done' OR status like 'failed') AND stored = 1 AND available = 1;"

#endif  //__OPH_QUERY_H

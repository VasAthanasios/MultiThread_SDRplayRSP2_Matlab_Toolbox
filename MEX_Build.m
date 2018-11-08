%   MEX build command.
% 
%   In case the included MEX does not work (e.g. Windows x86) replace
%   the files (.lib .dll) in folder rsp_lib with the x86 and run the
%   following command.
%
%   For any questions or assistance you can find me at, 
%   avasileiadis1@sheffield.ac.uk.
%
%   Vasileiadis Athanasios, 08 11 2018
%
mex sdrplayMT_mex.c rsp_lib\mir_sdr_api_0.lib rsp_lib\mir_sdr_api_1.lib rsp_lib\mir_sdr_api_2.lib rsp_lib\mir_sdr_api_3.lib;

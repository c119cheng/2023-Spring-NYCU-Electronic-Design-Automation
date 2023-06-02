./KF-ATPG/KF_ATPG -atpg NR -cir ./KF-ATPG/example/c17.bench -path_not ./KF-ATPG/example/c17.path_not -output ./case1_ans/my_c17_NR.pttn
./KF-ATPG/KF_ATPG -atpg R -cir ./KF-ATPG/example/c17.bench -path_not ./KF-ATPG/example/c17.path_not -output ./case1_ans/my_c17_R.pttn
./KF-ATPG/KF_ATPG -atpg NR -cir ./KF-ATPG/example/s27.bench -path_not ./KF-ATPG/example/s27.path_not -output ./case2_ans/my_s27_NR.pttn
./KF-ATPG/KF_ATPG -atpg R -cir ./KF-ATPG/example/s27.bench -path_not ./KF-ATPG/example/s27.path_not -output ./case2_ans/my_s27_R.pttn

diff ./case1_ans/my_c17_NR.pttn ./case1_ans/c17_NR.pttn
diff ./case1_ans/my_c17_R.pttn ./case1_ans/c17_R.pttn
diff ./case2_ans/my_s27_NR.pttn ./case2_ans/s27_NR.pttn
diff ./case2_ans/my_s27_R.pttn ./case2_ans/s27_R.pttn
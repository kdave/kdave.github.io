set terminal png transparent size 640,240
set size 1.0,1.0

set terminal png transparent size 640,480
set output 'commits_by_author.png'
set key left top
set yrange [0:]
set xdata time
set timefmt "%s"
set format x "%Y-%m-%d"
set grid y
set ylabel "Commits"
set xtics rotate
set bmargin 6
plot 'commits_by_author.dat' using 1:2 title "David Sterba" w lines, 'commits_by_author.dat' using 1:3 title "Qu Wenruo" w lines, 'commits_by_author.dat' using 1:4 title "Chris Mason" w lines, 'commits_by_author.dat' using 1:5 title "Josef Bacik" w lines, 'commits_by_author.dat' using 1:6 title "Anand Jain" w lines, 'commits_by_author.dat' using 1:7 title "Wang Shilong" w lines, 'commits_by_author.dat' using 1:8 title "Eric Sandeen" w lines, 'commits_by_author.dat' using 1:9 title "Gui Hecheng" w lines, 'commits_by_author.dat' using 1:10 title "Zhao Lei" w lines, 'commits_by_author.dat' using 1:11 title "Zach Brown" w lines, 'commits_by_author.dat' using 1:12 title "Lu Fengqi" w lines, 'commits_by_author.dat' using 1:13 title "Satoru Takeuchi" w lines, 'commits_by_author.dat' using 1:14 title "Su Yue" w lines, 'commits_by_author.dat' using 1:15 title "Stefan Behrens" w lines, 'commits_by_author.dat' using 1:16 title "Jeff Mahoney" w lines, 'commits_by_author.dat' using 1:17 title "Filipe David Borba Manana" w lines, 'commits_by_author.dat' using 1:18 title "Liu Bo" w lines, 'commits_by_author.dat' using 1:19 title "Goffredo Baroncelli" w lines, 'commits_by_author.dat' using 1:20 title "Miao Xie" w lines, 'commits_by_author.dat' using 1:21 title "Tsutomu Itoh" w lines

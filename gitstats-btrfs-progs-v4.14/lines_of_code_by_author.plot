set terminal png transparent size 640,240
set size 1.0,1.0

set terminal png transparent size 640,480
set output 'lines_of_code_by_author.png'
set key left top
set yrange [0:]
set xdata time
set timefmt "%s"
set format x "%Y-%m-%d"
set grid y
set ylabel "Lines"
set xtics rotate
set bmargin 6
plot 'lines_of_code_by_author.dat' using 1:2 title "David Sterba" w lines, 'lines_of_code_by_author.dat' using 1:3 title "Qu Wenruo" w lines, 'lines_of_code_by_author.dat' using 1:4 title "Chris Mason" w lines, 'lines_of_code_by_author.dat' using 1:5 title "Josef Bacik" w lines, 'lines_of_code_by_author.dat' using 1:6 title "Anand Jain" w lines, 'lines_of_code_by_author.dat' using 1:7 title "Wang Shilong" w lines, 'lines_of_code_by_author.dat' using 1:8 title "Eric Sandeen" w lines, 'lines_of_code_by_author.dat' using 1:9 title "Gui Hecheng" w lines, 'lines_of_code_by_author.dat' using 1:10 title "Zhao Lei" w lines, 'lines_of_code_by_author.dat' using 1:11 title "Zach Brown" w lines, 'lines_of_code_by_author.dat' using 1:12 title "Lu Fengqi" w lines, 'lines_of_code_by_author.dat' using 1:13 title "Satoru Takeuchi" w lines, 'lines_of_code_by_author.dat' using 1:14 title "Su Yue" w lines, 'lines_of_code_by_author.dat' using 1:15 title "Stefan Behrens" w lines, 'lines_of_code_by_author.dat' using 1:16 title "Jeff Mahoney" w lines, 'lines_of_code_by_author.dat' using 1:17 title "Filipe David Borba Manana" w lines, 'lines_of_code_by_author.dat' using 1:18 title "Liu Bo" w lines, 'lines_of_code_by_author.dat' using 1:19 title "Goffredo Baroncelli" w lines, 'lines_of_code_by_author.dat' using 1:20 title "Miao Xie" w lines, 'lines_of_code_by_author.dat' using 1:21 title "Tsutomu Itoh" w lines

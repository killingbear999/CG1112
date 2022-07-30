set size square 1,1     				#set the aspect ratio to 1:1
set object circle at 160,160 size 30  	#draw 3 concentric circle centered on 160,160
set object circle at 160,160 size 20 
set object circle at 160,160 size 10 
set label "20 cm" at 155,150 boxed		#label the 3 circles
set label "30 cm" at 155,140 boxed		#find out the actual distance
set label "40 cm" at 155,130 boxed
plot [130:190] [130:190] "lidar_reading.dat" using 1:2 with points pointtype 4
pause 3
reread

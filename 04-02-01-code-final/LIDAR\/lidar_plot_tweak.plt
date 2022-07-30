set size square 1,1     				#set the aspect ratio to 1:1
set object circle at 40,40 size 40  	#draw 3 concentric circle centered on 40,40
set object circle at 40,40 size 30 
set object circle at 40,40 size 20 
set label "X cm" at 40,40 boxed		#label the 3 circles
set label "Y cm" at 30,20 boxed		#find out the actual distance
set label "Z cm" at 35,25 boxed
plot [0:80] [0:80] "lidar_reading.dat" using 1:2 with points pointtype 4

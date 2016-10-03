default:
	scons -f arduino.sconstruct

up2twix:
	scp romreader.* twix:~/projects/kurzweil/romreader/

up2ard:
	./pulsedtr.py /dev/ttyUSB0
	scons -f arduino.sconstruct upload
	./pulsedtr.py /dev/ttyUSB0
	sh avrtest.sh	
	miniterm.py /dev/ttyUSB0 -b 115200

clean:
	scons -f arduino.sconstruct -c

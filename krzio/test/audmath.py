#!/usr/bin/env python


from math import *
import turtle

def logbase(base,inp):
  return log(inp)/log(base)

def dbpersec(x,secs=1):
 return pow(pow(10.0,x/20.0),secs)

def dbpersample(x,srate):
 return dbpertim(x,1/srate)

def lin2cents(lin):
  return logbase(2,lin)*1200

def cents2lin(cents):
	return pow( 2.0, (cents/1200.0) );

tc = 16.3515978307

def midi2frq(x):
  return (.5*tc)*pow(2,x*100/1200.0)



# HPITCH is # cents from 0 to get to max SR



#output = inputCtrl * (minDepth - ((maxDepth - minDepth) * depthCtrl));

def divclk(x):
	return 1000000000.0/x

def testfrq(rk,clkval,hpitch,lpsize=None):
	sr = divclk(clkval)
	srrat = 1.0*(48000.0/sr)
	rkcents = rk*100
	delc = hpitch-rkcents
	frqerc = lin2cents(srrat)
	padj = frqerc-delc
	print "clkval<%d> SR<%d> SRrat<%f> HPcents<%d> RKcents<%d> DELcents<%d> SRratc<%d> PCHADJUST<%d>" % (clkval,sr,srrat,hpitch,rkcents,delc,frqerc,padj)

testfrq( 32, 52083, 4799 )
testfrq( 35, 52080, 5106 )
testfrq( 39, 52083, 5498 )
testfrq( 44, 52080, 6015 )
testfrq( 46, 29761, 5210 )
testfrq( 52, 46294, 6613 )
testfrq( 56, 24990, 5912, (4184341-4184149) )
testfrq( 60, 20833, 6000 )
testfrq( 60, 20832, 6000 )
testfrq( 63, 24990, 6610, (4184470-4184342) )
testfrq( 70, 24990, 7312, (4191292-4191207) )
testfrq( 76, 52083, 9238 )
testfrq( 77, 24990, 8024, (4192644-4192588) )
testfrq( 81, 41666, 9325 )
testfrq( 86, 41664, 9823 )
testfrq( 91, 24990, 9383, (4184677-4184652) )
testfrq( 98, 24990, 10118, (4184694-4184678) )
testfrq( 100, 41661, 11222 )
testfrq( 123, 25111, 12623, (4184739-4184736) )
testfrq( 127, 26573, 13121, (4184742-4184740) )

#GRANDPIANO
#rootkey   #SR        #HPITCH   #sr_clkval
#  32      19200.123   4799     52083
#  35      19201.228   5106     52080
#  39      19200.123   5498     52083
#  44      19201.228   6015     52080
#  52      21601.072   6613     46294
#  60      19201.228   7605     52080
#  76      21597.339   9016     46302
#  81      24000.384   9325     41666
#  86      24001.535   9823     41664
# 100      24003.263  11222     41661

#HARDRHODES
#  41      16800.228   5920     59523
#  54      19198.279   6987     52088
#  76      19200.123   9238     52083

#SOFTRHODES
#  41      16804.181   5911     59509
#  81      21595.008   9482     46307

#voices
#  46      33601.019   5210     29761
#  99      33601.019  10522     29761

#drums
#  60      48000.769   6000     20833
#  60      48003.070   6000     20832

#clave
#  60      33601.019   6617     29761

#clav (single cylcles)
#  127    37632.183    13121    26573

#bell
#  98      48000.769   9800  

#jazzg
#  49      24001.535   6119 
############################################
#  32      19200.123   4799(x12bf) 52083
#  76      19200.123   9238(x2416) 52083
#  76      21597.339   9016(x2338) 46302

#  46      33601.019   5210     29761
#  99      33601.019  10522     29761
# 100      24003.263  11222     41661


#smallest clock divisor ~= 20683 (48348.88555818788 khz)

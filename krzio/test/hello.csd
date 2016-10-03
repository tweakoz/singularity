<CsoundSynthesizer>
<CsOptions>
-odac    ;;;realtime audio out
</CsOptions>
<CsInstruments>

sr = 44100
ksmps = 32
nchnls = 2
0dbfs  = 1


instr 1 ; layer 1

	kkey = 60
	kx = p4-kkey
	kcps=cpsmidinn(kkey)
	kcpsx=cpsmidinn(kx-48)
	printk2 kkey
	printk2 kcps
	printk2 kcpsx
	kdbps = -6.0
	kattenps pow 10,kdbps/20
	ktim timeinsts
	katten pow kattenps,ktim
	asampleout loscil .25*katten, kcpsx, 1, 1, 1

	awrapped wrap asampleout, 0, .1

	;ao2 oscils .7, 10*p4, 0, 2
    	 outs asampleout, asampleout

endin

instr 2 ; layer 2

	kkey = 60
	kx = p4-kkey
	kcps=cpsmidinn(kkey)
	kcpsx=cpsmidinn(kx-48)
	printk2 kkey
	printk2 kcps
	printk2 kcpsx
	kdbps = -6.0
	kattenps pow 10,kdbps/20
	ktim timeinsts
	katten pow kattenps,ktim
	asampleout loscil .25*katten, kcpsx, 1, 1, 1

	awrapped wrap asampleout, 0, .1

	;ao2 oscils .7, 10*p4, 0, 2
    	 outs asampleout, asampleout

endin

instr 3 ; layer 3

	kkey = 60
	kx = p4-kkey
	kcps=cpsmidinn(kkey)
	kcpsx=cpsmidinn(kx-48)
	printk2 kkey
	printk2 kcps
	printk2 kcpsx
	kdbps = -6.0
	kattenps pow 10,kdbps/20
	ktim timeinsts
	katten pow kattenps,ktim
	asampleout loscil .25*katten, kcpsx, 1, 1, 1, 8117*2, 20535*2

	awrapped wrap asampleout, 0, .1

	;ao2 oscils .7, 10*p4, 0, 2
    	 outs asampleout, asampleout

endin

</CsInstruments>
<CsScore>

f 1 0 0 1 "samples/001_Grand_Piano_0.aiff" 0 0 0
f 2 0 0 1 "samples/005_Voices_0.aiff" 0 0 0
f 3 0 0 1 "samples/006_Ensemble_Strings_0.aiff" 0 0 0

i 1 0 5 60	; i=1 t=0 d=.9 p4=0

;i 1 .5 5 61	; i=1 t=1 d=.9 p4=0
;i 1 1 5 62	; i=1 t=2 d=.9 p4=0
;i 1 1.5 5 63	; i=1 t=3 d=2 p4=2
;i 1 2 5 64	; i=1 t=3 d=2 p4=2
;i 1 2.5 5 65	; i=1 t=3 d=2 p4=2
;i 1 3 5 66	; i=1 t=3 d=2 p4=2
;i 1 3.5 5 67	; i=1 t=3 d=2 p4=2
;i 1 4 5 68	; i=1 t=3 d=2 p4=2
;i 1 4.5 5 69	; i=1 t=3 d=2 p4=2
;i 1 5 5 70	; i=1 t=3 d=2 p4=2
;i 1 5.5 5 71	; i=1 t=3 d=2 p4=2
;i 1 6 5 72	; i=1 t=3 d=2 p4=2
e
</CsScore>
</CsoundSynthesizer>

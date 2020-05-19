from rsf.proj import *

# Preprocess data
dat={}
for i in range(4):
    dat[i] = 'Anisotropic_FD_Model_Shots_part%d'%(i+1)
    Flow(dat[i]+'.sgy',dat[i]+'.sgy.gz','gunzip -c $SOURCE ',stdin=0)
    Flow(dat[i],dat[i]+'.sgy','segyread endian=y')

Flow('shot',dat.values(),'cat axis=2 ${SOURCES[1:4]} | put n2=800 d2=0.0125 o2=-10.025 n3=1641 d3=0.05 o3=0 label2="Offset" unit2="km" label3="Shot" unit3="km"')

#
## Prepare TTI parameters
## ######################
## Download from http://www.freeusp.org/2007_BP_Ani_Vel_Benchmark/
#tgz = 'ModelParams.tar.gz'
##Fetch(tgz,'BP',top=os.environ.get('DATAPATH'),server='local')
#
#pars = Split('epsilon delta vp theta')
#sgy = {}
#for par in pars:
#    sgy[par] = os.path.join('ModelParams',par.capitalize() + '_Model.sgy')
#Flow(sgy.values(),tgz,'gunzip -c $SOURCE | tar -xvf -',stdin=0,stdout=-1)
#
#for par in pars:
#    Flow([par,par+'.asc',par+'.bin'],sgy[par],
#         '''
#         segyread hfile=${TARGETS[1]} bfile=${TARGETS[2]} read=d |
#         put
#         o2=0 d2=0.00625 label2=Distance unit2=km
#         o1=0 d1=0.00625 label1=Depth unit1=km %s 
#         ''' % ('','| scale dscale=0.001')[par=='vp'])
#
## Vx velocity, eta and smoothed theta
#Flow('vx','vp epsilon','math e=${SOURCES[1]} output="input*sqrt(2*e+1)"')
#Flow('eta','epsilon delta','''
#	math d=${SOURCES[1]}
#	output="((1.+2.*input)/(1.+2.*d)-1.)/2."
#	''')
#Flow('theta0','theta','smooth rect1=20 rect2=30')
#
#for par in ('vp','vx','eta','theta0'):
#    Flow(par+'j',par,'window j1=3 j2=2 |pad2 left=802 right=761')
#    Result(par+'j',
#    '''
#	grey color=j barlabel=%s scalebar=y
#    labelsz=4 titlesz=5 barreverse=y
#    wanttitle=n allpos=%d bias=%g barunit=%s
#    parallel2=n screenratio=0.4
#    ''' % (par.capitalize(),
#            par != 'theta',
#            (0,1.5)[par=='vp'],
#            ('','km/s')[par=='vp']))
#
## Source
## #####################
#nt=2301
#dt=0.004
#ddt=0.0005
#factor=dt/ddt
#nnt=(nt-1)*factor+1
#frequency=19.3
#
#Flow('source',None,
#		'''
#		spike n1=%d d1=%g k1=%d |
#		ricker1 frequency=%g |
#		scale rscale=100
#		''' %(nnt, ddt, 1./frequency/ddt+2*factor, frequency))
#Flow('real','source','math output=0.')
#Flow('imag','source','envelope hilb=y order=500|halfint |halfint |math output="input/2."')
#Flow('csource','real imag','cmplx ${SOURCES[1]} |window j1=%d' %factor)
#Result('csource','imag |graph max1=1. label2=Amplitude label1=Time title=')
#
## Lowrank decomposition
## #####################
#nb=60
#rightf='./mat/right'
#leftf='./mat/left'
#vpf='./vp/vp'
#vxf='./vp/vx'
#etaf='./vp/eta'
#thetaf='./vp/theta'
#fftf='./fft/fft'
#
#ns=1641
#ds=0.05
#nds=4
#
#old=['vpj','vxj','etaj','theta0j']
#for i in range(ns):
#    vz=vpf+'%d' %(i+1)
#    vx=vxf+'%d' %(i+1)
#    eta=etaf+'%d' %(i+1)
#    theta=thetaf+'%d' %(i+1)
#    new=[vz,vx,eta,theta]
#    for j in range(4):
#        Flow(new[j],old[j],
#		    '''
#			window f2=%d n2=%d |
#			pad2 bottom=%d top=%d left=%d right=%d
#			''' %(i*nds,1301,nb,nb,nb,nb))
#    
#    fft=fftf+'%d' %(i+1)
#    Flow(fft,vz,'rtoc |fft3 axis=1 pad=1 |fft3 axis=2 pad=1')
#
#    left=leftf+'%d' %(i+1)
#    right=rightf+'%d' %(i+1)
#    Flow([right,left],[vz,vx,eta,theta,fft],
#        '''
#        zanisolr2 seed=2013 dt=%g velx=${SOURCES[1]} eta=${SOURCES[2]}
#        theta=${SOURCES[3]} fft=${SOURCES[4]} vels=${SOURCES[1]} left=${TARGETS[1]} npk=50
#		eps=1e-4 os=y sub=n mode=0 abc=2 approx=2
#        nbt=%d nbb=%d nbl=%d nbr=%d ct=%g cb=%g cl=%g cr=%g
#        ''' %(dt, nb, nb, nb, nb, 0.01, 0.01, 0.01, 0.01))
#
## Data
## ########################
#Flow('data','shot.hh',
#    '''
#	remap1 d1=0.004 n1=2301 |
#	mutter v0=3 t0=0.15 |
#	addimag
#	''')
#
#Flow('testvp','vp','window f2=4000 n2=1701')
#Flow('testdata','shot.hh',
#    '''
#	window f3=1000 j3=10 |
#	window n3=11 |
#	remap1 d1=0.004 n1=2301 |
#	rtoc
#	''')

# RTM images
# ##########
# export OMP_NUM_THREADS=12
# ibrun tacc_affinity sfmpilrrtm_ts --input=testdata.rsf --output=img1.rsf Fimg2=img2.rsf Fsrc=csource.rsf Fpadvel=testvp.rsf verb=y taper=4 nb=60 nds=40 gpz=0 spx=802 spz=0 rnx=1301 ntau=201 dtau=0.004 tau0=-0.4
# ibrun tacc_affinity sfmpilrrtm_ts --input=data.rsf --output=img1.rsf Fimg2=img2.rsf Fsrc=csource.rsf Fpadvel=vp.rsf verb=n taper=4 nb=60 nds=4 gpz=0 spx=802 spz=0 rnx=1301 ntau=201 dtau=0.004 tau0=-0.4 

Flow('bpcube','img1',
    '''
	window f2=802 n2=6298 |
	laplac |pow pow1=1.3
	''')
Result('bpcube',
    '''
	byte gainpanel=all |
	grey3 flat=n title= 
	frame1=450 frame2=3000 frame3=100
	point1=0.85 point2=0.73 labelsz=4
	screenratio=0.6 screenht=8 label3="Time Shift"
	''')
Result('bptti-img','bpcube',
    '''
	window n3=1 f3=100 |
	grey title= screenratio=0.2 color=g pclip=98 
	''')
x1=15
y1=3
x2=34
y2=8.00625
Flow('frame1.asc',None,'echo %s n1=10 data_format=ascii_float in=$TARGET'% string.join(map(str,(x1,y1, x1,y2, x2,y2, x2,y1, x1,y1))))
Plot('frame1','frame1.asc',
		'''
		dd type=complex form=native |
		graph min2=0 max2=11.25 min1=0 max1=78.7125 pad=n plotfat=12 plotcol=6
		wantaxis=n wanttitle=n yreverse=y scalebar=n screenratio=0.2
		''')
Result('bptti-image','Fig/bptti-img frame1','Overlay')

# Portion 1
# #################
Flow('zone1','bpcube','window min1=2 max1=10 min2=13 max2=36')
Flow('padzone1','zone1','padzero scalex=2 scalez=3')

## Padding by interpolation
#Flow('coord',None,
#    '''
#	math n1=2 n2=1279 n3=3681
#	d1=1 o1=0
#	d2=0.00625 o2=2.00625
#	d3=0.00625 o3=13
#	output="(1-x1)*x2+x1*x3"
#	''')
#Flow('lpadzone1','zone1 coord',
#    '''
#	inttest2 nw=4 interp=spl coord=${SOURCES[1]}
#	''')

nb=60
dt=0.001
for par in ('vp','vx','eta','theta0'):
    Flow(par+'1',par,'window min1=2.00625 max1=9.99375 min2=13 max2=36')
    Flow(par+'1pad',par+'1','pad2 top=%d bottom=%d left=%d right=%d' %(nb, nb, nb, nb))
Flow('fft1','vp1pad','rtoc |fft3 axis=1 pad=2 |fft3 axis=2 pad=1')
Flow('right1 left1','vp1pad vx1pad eta1pad theta01pad fft1',
    '''
	anisolr2 seed=2013 dt=%g velx=${SOURCES[1]} eta=${SOURCES[2]} theta=${SOURCES[3]}
	fft=${SOURCES[4]} left=${TARGETS[1]} npk=60 eps=1e-5
	''' %dt)

# Portion 1: Time-shift interpolation
# <padzone1.rsf sfmpictshiftlr left=left1.rsf right=right1.rsf cmplx=y pad1=2>czone1.rsf

Flow('sczone1','czone1',
    '''
	window min1=3 max1=8.00625 min2=15 max2=34 |
	stack axis=3
	''')

Result('sczone1','grey title= screenratio=0.45 color= pclip=99')
Result('zone1','window f3=100 n3=1 min1=3 max1=8 min2=15 max2=34 |grey title= screenratio=0.45 color= pclip=99')

#Flow('zone1-dense',zone, ''' window min1=3 max1=8.00625 min2=15 max2=34 | stack axis=3
#	''')
#Flow('zone1-coarse','data','window f3=100 n3=1 min1=3 max1=8 min2=15 max2=34')
#
#Result('zone1-dense','grey title= screenratio=0.45 color= pclip=99')
#Result('zone1-coarse','grey title= screenratio=0.45 color= pclip=99')
#
#Flow('zone2-dense',zone,
#    '''
#	window min1=3 max1=8.00625 min2=15 max2=30 |
#	stack axis=3
#	''')
#Flow('zone2-coarse','data','window f3=100 n3=1 min1=3 max1=8 min2=15 max2=30')
#
#Result('zone2-dense','grey title= screenratio=0.45 color= pclip=97')
#Result('zone2-coarse','grey title= screenratio=0.45 color= pclip=97')

# Portion 1: Time shift gathers
Flow('bdata','bpcube','window min1=3 max1=8 min2=15 max2=34')
Result('btg1','bdata','window n2=1 min2=20 |grey title="x=20km" screenratio=2 labelsz=5 pclip=98')
Result('btg2','bdata','window n2=1 min2=25 |grey title="x=25km" screenratio=2 labelsz=5 pclip=98')
Result('btg3','bdata','window n2=1 min2=30 |grey title="x=30km" screenratio=2 labelsz=5 pclip=98')

Flow('adata','czone1','window min1=3 max1=8.00625 min2=15 max2=34')
Result('atg1','adata','window n2=1 min2=20 |grey title="x=20km" screenratio=2 labelsz=5 pclip=98')
Result('atg2','adata','window n2=1 min2=25 |grey title="x=25km" screenratio=2 labelsz=5 pclip=98')
Result('atg3','adata','window n2=1 min2=30 |grey title="x=30km" screenratio=2 labelsz=5 pclip=98')
# scons Fig/btg1.vpl Fig/btg2.vpl Fig/btg3.vpl Fig/atg1.vpl Fig/atg2.vpl Fig/atg3.vpl

# Portion 1: Target areas
x1=15.07
y1=3.05
x2=21
y2=5
Flow('frame2.asc',None,'echo %s n1=10 data_format=ascii_float in=$TARGET'% string.join(map(str,(x1,y1, x1,y2, x2,y2, x2,y1, x1,y1))))
Plot('frame2','frame2.asc',
		'''
		dd type=complex form=native |
		graph min2=3 max2=8.00625 min1=15 max1=34 pad=n plotfat=10 plotcol=6
		wantaxis=n wanttitle=n yreverse=y scalebar=n screenratio=0.45
		''')

x1=28
y1=6
x2=33.96
y2=7.97
Flow('frame3.asc',None,'echo %s n1=10 data_format=ascii_float in=$TARGET'% string.join(map(str,(x1,y1, x1,y2, x2,y2, x2,y1, x1,y1))))
Plot('frame3','frame3.asc',
		'''
		dd type=complex form=native |
		graph min2=3 max2=8.00625 min1=15 max1=34 pad=n plotfat=10 plotcol=6
		wantaxis=n wanttitle=n yreverse=y scalebar=n screenratio=0.45
		''')
Result('bptti-targeto','Fig/zone1 frame2 frame3','Overlay')
Result('bptti-target','Fig/sczone1 frame2 frame3','Overlay')

# Portion 1: Four parts
Result('zone1-portion1','zone1',
		'''
		window n3=1 f3=100 min1=3 max1=5 min2=15 max2=21 |
		grey title= screenratio=0.35 color=g pclip=98 labelsz=12
		''')
Result('czone1-portion1','sczone1',
		'''
		window min1=3 max1=5 min2=15 max2=21 |
		grey title= screenratio=0.35 color=g pclip=98 labelsz=12
		''')

Result('zone1-portion2','zone1',
		'''
		window n3=1 f3=100 min1=6 max1=8 min2=28 max2=34 |
		grey title= screenratio=0.35 color=g pclip=98 labelsz=12
		''')
Result('czone1-portion2','sczone1',
		'''
		window min1=6 max1=8 min2=28 max2=34 |
		grey title= screenratio=0.35 color=g pclip=98 labelsz=12
		''')
# scons Fig/zone1-portion1.vpl Fig/czone1-portion1.vpl Fig/zone1-portion2.vpl Fig/czone1-portion2.vpl

# Portion 2
# ############################
Flow('zone2','bpcube','window min1=2 max1=11 min2=62 max2=78')
Flow('padzone2','zone2','padzero scalex=2 scalez=3')

## Pad by interpolation
#Flow('coord',None,
#    '''
#	math n1=2 n2=1441 n3=2561
#	d1=1 o1=0
#	d2=0.00625 o2=2.00625
#	d3=0.00625 o3=62
#	output="(1-x1)*x2+x1*x3"
#	''')
#Flow('lpadzone2','zone2 coord',
#    '''
#	inttest2 nw=4 interp=spl coord=${SOURCES[1]}
#	''')

nb=60
dt=0.001
for par in ('vp','vx','eta','theta0'):
    Flow(par+'2',par,'window min1=2.00625 max1=11.00625 min2=62 max2=78')
    Flow(par+'2pad',par+'2','pad2 top=%d bottom=%d left=%d right=%d' %(nb, nb, nb, nb))
Flow('fft2','vp2pad','rtoc |fft3 axis=1 pad=2 |fft3 axis=2 pad=1')
Flow('right2 left2','vp2pad vx2pad eta2pad theta02pad fft2',
    '''
	anisolr2 seed=2013 dt=%g velx=${SOURCES[1]} eta=${SOURCES[2]} theta=${SOURCES[3]}
	fft=${SOURCES[4]} left=${TARGETS[1]} npk=60 eps=1e-5
	''' %dt)

# Portion 2: Time-shift interpolation
# <padzone2.rsf sfmpictshiftlr left=left2.rsf right=right2.rsf cmplx=y pad1=2>czone2.rsf

Flow('sczone2','czone2',
    '''
	window min1=3.99375 max1=6.99375 min2=66 max2=76 |
	stack axis=3
	''')
Result('sczone2','grey title= screenratio=0.5 screenht=6.5 labelsz=6 min1=4 max1=7')
Result('zone2','window f3=100 n3=1 min1=4 max1=7 min2=66 max2=76 |grey title= screenratio=0.5 screenht=6.5 labelsz=6')

# Representative time-shift gathers
####################################
Result('tg1','bpcube','window n2=1 min2=10 |grey title="x=10km" screenratio=1.5 labelsz=5')
Result('tg2','bpcube','window n2=1 min2=20 |grey title="x=20km" screenratio=1.5 labelsz=5')
Result('tg3','bpcube','window n2=1 min2=30 |grey title="x=30km" screenratio=1.5 labelsz=5')
Result('tg4','bpcube','window n2=1 min2=40 |grey title="x=40km" screenratio=1.5 labelsz=5')
Result('tg5','bpcube','window n2=1 min2=50 |grey title="x=50km" screenratio=1.5 labelsz=5')
Result('tg6','bpcube','window n2=1 min2=60 |grey title="x=60km" screenratio=1.5 labelsz=5')
Result('tg7','bpcube','window n2=1 min2=70 |grey title="x=70km" screenratio=1.5 labelsz=5')
# scons Fig/tg1.vpl Fig/tg2.vpl Fig/tg3.vpl Fig/tg4.vpl Fig/tg5.vpl Fig/tg6.vpl Fig/tg7.vpl

End()

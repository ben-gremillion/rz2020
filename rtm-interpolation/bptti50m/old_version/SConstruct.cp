from rsf.proj import *

# Prepare TTI parameters
# ######################
# Download from http://www.freeusp.org/2007_BP_Ani_Vel_Benchmark/
tgz = 'ModelParams.tar.gz'
#Fetch(tgz,'BP',top=os.environ.get('DATAPATH'),server='local')

pars = Split('epsilon delta vp theta')
sgy = {}
for par in pars:
    sgy[par] = os.path.join('ModelParams',par.capitalize() + '_Model.sgy')
Flow(sgy.values(),tgz,'gunzip -c $SOURCE | tar -xvf -',stdin=0,stdout=-1)

for par in pars:
    Flow([par,par+'.asc',par+'.bin'],sgy[par],
         '''
         segyread hfile=${TARGETS[1]} bfile=${TARGETS[2]} read=d |
         put
         o2=0 d2=0.00625 label2=Distance unit2=km
         o1=0 d1=0.00625 label1=Depth unit1=km %s  |
		 window j1=2 j2=2
         ''' % ('','| scale dscale=0.001')[par=='vp'])
Result('vp','grey title= screenratio=0.5 color=j mean=y scalebar=y barreverse=y barlabel=Velocity barunit=km/s')

# Vx velocity, eta and smoothed theta
Flow('vx','vp epsilon','math e=${SOURCES[1]} output="input*sqrt(2*e+1)"')
Flow('eta','epsilon delta','''
	math d=${SOURCES[1]}
	output="((1.+2.*input)/(1.+2.*d)-1.)/2."
	''')
Flow('theta0','theta','smooth rect1=20 rect2=30')

# 200+50 66+100 201+150
for par in ('vp','vx','eta','theta0'):
    Flow(par+'j',par,'window j1=4 j2=4 |pad2 left=250 right=166')
    Result(par+'j',
    '''
	grey color=j barlabel=%s scalebar=y
    barreverse=y wanttitle=n allpos=%d bias=%g barunit=%s
    parallel2=n screenratio=0.4
    ''' % (par.capitalize(),
            par != 'theta',
            (0,1.5)[par=='vp'],
            ('','km/s')[par=='vp']))

# Source
# #####################
nt=4601
dt=0.002
ddt=0.0005
factor=dt/ddt
nnt=(nt-1)*factor+1
frequency=19.3

Flow('source',None,
		'''
		spike n1=%d d1=%g k1=%d |
		ricker1 frequency=%g |
		scale rscale=100
		''' %(nnt, ddt, 1./frequency/ddt+4*factor, frequency))
Flow('real','source','math output=0.')
Flow('imag','source','envelope hilb=y order=500|halfint |halfint |math output="input/2."')
Flow('csource','real imag','cmplx ${SOURCES[1]} |window j1=%d' %factor)
Result('csource','imag |graph max1=1. label2=Amplitude label1=Time title=')

# Lowrank decomposition for RTM
# ##############################
nb=60
rightf='./mat/right'
leftf='./mat/left'
vpf='./vp/vp'
vxf='./vp/vx'
etaf='./vp/eta'
thetaf='./vp/theta'
fftf='./fft/fft'

ns=411
ds=0.2
nds=4
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
#			''' %(i*nds,351,nb,nb,nb,nb))
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

# Data interpolation
# ####################
# <shot.hh sfwindow j3=4 |sfwindow f2=2 j2=4 >shot4.rsf
#Flow('data','shot4',
#    '''
#	remap1 d1=0.002 n1=4601 |
#	mutter v0=3 t0=0.15 |
#	addimag
#	''')

# RTM images
# ##########
# 200+50 66+100 201+150
# export OMP_NUM_THREADS=12
# ibrun tacc_affinity sfmpilrrtm_ts --input=data.rsf --output=img1.rsf Fimg2=img2.rsf Fsrc=csource.rsf Fpadvel=vpj.rsf verb=n repeat=1 taper=4 nb=60 nds=4 gpz=0 spx=250 spz=0 rnx=351 nr0=50 ntau=201 dtau=0.002 tau0=-0.2

Flow('bpcube','img1',
    '''
	window f2=250 n2=1575 |
	laplac |pow pow1=1.3
	''')
Result('bpcube',
    '''
	byte gainpanel=all |
	grey3 flat=n title= 
	frame1=113 frame2=750 frame3=100
	point1=0.85 point2=0.73
	screenratio=0.6 label3="Time Shift"
	''')
Result('bptti-img','bpcube',
    '''
	window n3=1 f3=100 |
	grey title= screenratio=0.5 pclip=98 
	''')

# Target areas
x1=15
y1=2.5
x2=30
y2=8
Flow('frame1.asc',None,'echo %s n1=10 data_format=ascii_float in=$TARGET'% string.join(map(str,(x1,y1, x1,y2, x2,y2, x2,y1, x1,y1))))
Plot('frame1','frame1.asc',
		'''
		dd type=complex form=native |
		graph min2=0 max2=11.25 min1=0 max1=78.7 pad=n plotfat=10 plotcol=6
		wantaxis=n wanttitle=n yreverse=y scalebar=n screenratio=0.5
		''')
x1=40
y1=6
x2=60
y2=10
Flow('frame2.asc',None,'echo %s n1=10 data_format=ascii_float in=$TARGET'% string.join(map(str,(x1,y1, x1,y2, x2,y2, x2,y1, x1,y1))))
Plot('frame2','frame2.asc',
		'''
		dd type=complex form=native |
		graph min2=0 max2=11.25 min1=0 max1=78.7 pad=n plotfat=10 plotcol=6
		wantaxis=n wanttitle=n yreverse=y scalebar=n screenratio=0.5
		''')
Result('bptti-image','Fig/bptti-img frame1','Overlay')
#Result('bptti-image','Fig/bptti-img frame1 frame2','Overlay')

# Portion 1 #######
# #################
Flow('zone1','bpcube','window min1=1. max1=10 min2=12 max2=33')
Flow('padzone1','zone1','padzero scalex=4 scalez=4')
Flow('interzone1','zone1','sinc n1=721 d1=0.0125 o1=1 |transp |sinc n1=1681 d1=0.0125 o1=12 |transp')

nb=60
dt=0.001
for par in ('vp','vx','eta','theta0'):
    Flow(par+'1',par,'window min1=1 n1=721 min2=12 n2=1681')
    Flow(par+'1pad',par+'1','pad2 top=%d bottom=%d left=%d right=%d' %(nb, nb, nb, nb))
Flow('fft1','vp1pad','rtoc |fft3 axis=1 pad=1 |fft3 axis=2 pad=1')
Flow('right1 left1','vp1pad vx1pad eta1pad theta01pad fft1',
    '''
	anisolr2 seed=2013 dt=%g velx=${SOURCES[1]} eta=${SOURCES[2]} theta=${SOURCES[3]}
	fft=${SOURCES[4]} left=${TARGETS[1]} npk=60 eps=1e-5
	''' %dt)

# Portion 1: Time-shift interpolation
# <padzone1.rsf sfmpictshiftlr left=left1.rsf right=right1.rsf cmplx=y>cpadzone1.rsf
# <interzone1.rsf sfmpictshiftlr left=left1.rsf right=right1.rsf cmplx=y>cinterzone1.rsf

Flow('scpadzone1','cpadzone1',
    '''
	window min1=2.5 max1=8 min2=15 max2=30 |
	stack axis=3
	''')
Result('scpadzone1','grey title= screenratio=0.5 color= pclip=99')

Flow('scinterzone1','cinterzone1',
    '''
	window min1=2.5 max1=8 min2=15 max2=30 |
	stack axis=3
	''')
Result('scinterzone1','grey title= screenratio=0.5 color= pclip=99')
Result('interzone1','window f3=100 n3=1 min1=2.5 max1=8 min2=15 max2=30 |grey title= screenratio=0.5 color= pclip=99')
# scons Fig/scpadzone1.vpl Fig/scinterzone1.vpl Fig/interzone1.vpl

# Portion 1: Time shift gathers
Flow('bdata','interzone1','window min1=2.5 max1=8 min2=15 max2=30')
Result('btg1','bdata','window n2=1 min2=20 |grey label2="Time Shift" title="x=20km" screenratio=2 labelsz=5 pclip=98')
Result('btg2','bdata','window n2=1 min2=25 |grey label2="Time Shift" title="x=25km" screenratio=2 labelsz=5 pclip=98')
Result('btg3','bdata','window n2=1 min2=30 |grey label2="Time Shift" title="x=30km" screenratio=2 labelsz=5 pclip=98')

Flow('adata','cinterzone1','window min1=2.5 max1=8 min2=15 max2=30')
Result('atg1','adata','window n2=1 min2=20 |grey label2="Time Shift" title="x=20km" screenratio=2 labelsz=5 pclip=98')
Result('atg2','adata','window n2=1 min2=25 |grey label2="Time Shift" title="x=25km" screenratio=2 labelsz=5 pclip=98')
Result('atg3','adata','window n2=1 min2=30 |grey label2="Time Shift" title="x=30km" screenratio=2 labelsz=5 pclip=98')
# scons Fig/btg1.vpl Fig/btg2.vpl Fig/btg3.vpl Fig/atg1.vpl Fig/atg2.vpl Fig/atg3.vpl

# Portion 2 ##################
# ############################
# x1=40 y1=6 x2=60 y2=10
Flow('zone2','bpcube','window min1=4 max1=11 min2=38 max2=62')
Flow('padzone2','zone2','padzero scalex=4 scalez=4')
Flow('interzone2','zone2','sinc n1=561 d1=0.0125 o1=4 |transp |sinc n1=1921 d1=0.0125 o1=38 |transp')

nb=60
dt=0.001
for par in ('vp','vx','eta','theta0'):
    Flow(par+'2',par,'window min1=4 n1=561 min2=38 n2=1921')
    Flow(par+'2pad',par+'2','pad2 top=%d bottom=%d left=%d right=%d' %(nb, nb, nb, nb))
Flow('fft2','vp2pad','rtoc |fft3 axis=1 pad=1 |fft3 axis=2 pad=1')
Flow('right2 left2','vp2pad vx2pad eta2pad theta02pad fft2',
    '''
	anisolr2 seed=2013 dt=%g velx=${SOURCES[1]} eta=${SOURCES[2]} theta=${SOURCES[3]}
	fft=${SOURCES[4]} left=${TARGETS[1]} npk=60 eps=1e-5
	''' %dt)

# Portion 2: Time-shift interpolation
# <padzone2.rsf sfmpictshiftlr left=left2.rsf right=right2.rsf cmplx=y>cpadzone2.rsf
# <interzone2.rsf sfmpictshiftlr left=left2.rsf right=right2.rsf cmplx=y>cinterzone2.rsf

Flow('scpadzone2','cpadzone2',
    '''
	window min1=6 max1=10 min2=40 max2=60 |
	stack axis=3
	''')
Result('scpadzone2','grey title= screenratio=0.35 pclip=95')

Flow('scinterzone2','cinterzone2',
    '''
	window min1=6 max1=10 min2=40 max2=60 |
	stack axis=3
	''')
Result('scinterzone2','grey title= screenratio=0.35 pclip=95')
Result('interzone2','window f3=100 n3=1 min1=6 max1=10 min2=40 max2=60 |grey title= screenratio=0.35 pclip=95')
# scons Fig/scpadzone2.vpl Fig/scinterzone2.vpl Fig/interzone2.vpl

# Portion 2: Time shift gathers
Flow('bdata2','bpcube','window min1=6 max1=10 min2=40 max2=60')
Result('btg4','bdata2','window n2=1 min2=45 |grey title="x=45km" screenratio=2 labelsz=5 pclip=98')
Result('btg5','bdata2','window n2=1 min2=50 |grey title="x=50km" screenratio=2 labelsz=5 pclip=98')
Result('btg6','bdata2','window n2=1 min2=55 |grey title="x=55km" screenratio=2 labelsz=5 pclip=98')

Flow('adata2','cinterzone2','window min1=6 max1=10 min2=40 max2=60')
Result('atg4','adata2','window n2=1 min2=45 |grey title="x=45km" screenratio=2 labelsz=5 pclip=98')
Result('atg5','adata2','window n2=1 min2=50 |grey title="x=50km" screenratio=2 labelsz=5 pclip=98')
Result('atg6','adata2','window n2=1 min2=55 |grey title="x=55km" screenratio=2 labelsz=5 pclip=98')
# scons Fig/btg4.vpl Fig/btg5.vpl Fig/btg6.vpl Fig/atg4.vpl Fig/atg5.vpl Fig/atg6.vpl

# Different features
x1=43
y1=8.4
x2=44
y2=9.1
x3=49
y3=8.3
x4=48
y4=7.6
Flow('frame3.asc',None,'echo %s n1=10 data_format=ascii_float in=$TARGET'% string.join(map(str,(x1,y1, x2,y2, x3,y3, x4,y4, x1,y1))))
Plot('frame3','frame3.asc',
		'''
		dd type=complex form=native |
		graph min2=6 max2=10 min1=40 max1=60 pad=n plotfat=8 plotcol=6
		wantaxis=n wanttitle=n yreverse=y scalebar=n screenratio=0.35
		''')
x1=53
y1=8
x2=52.5
y2=8.5
x3=55.5
y3=9.8
x4=58
y4=9.8
x5=58
y5=9.3
x6=56.5
y6=9.3
Flow('frame4.asc',None,'echo %s n1=14 data_format=ascii_float in=$TARGET'% string.join(map(str,(x1,y1, x2,y2, x3,y3, x4,y4, x5,y5, x6,y6, x1,y1))))
Plot('frame4','frame4.asc',
		'''
		dd type=complex form=native |
		graph min2=6 max2=10 min1=40 max1=60 pad=n plotfat=8 plotcol=6
		wantaxis=n wanttitle=n yreverse=y scalebar=n screenratio=0.35
		''')

Result('scpadzone2p','Fig/scpadzone2 frame3 frame4','Overlay')
Result('scinterzone2p','Fig/scinterzone2 frame3 frame4','Overlay')
Result('interzone2p','Fig/interzone2 frame3 frame4','Overlay')
# scons Fig/scpadzone2p.vpl Fig/scinterzone2p.vpl Fig/interzone2p.vpl

End()

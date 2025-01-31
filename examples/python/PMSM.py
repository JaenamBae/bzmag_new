# Python PMSM Example for bzmagEdior

# Part I. 준비작업: 이 작업은 모든 경우에 대해서 필요함
# imort bzmagPy
# bzmagPy는 bzmagEditor의 기능을 파이선확장으로 만든 것임
import bzmagPy as bzmag

# get Modeler
# 프로그램 좌측 트리 구조에서 트리 노드가 내부적으로 특정 기능을 수행하는데
# 그 트리 노드를 가져오는 함수가 get() 함수임
# bzmagPy의 함수이므로 bzmag.get() 형태로 쓰임
modeler = bzmag.get('/sys/modeler')

# Part II. 모델링
# 모델러를 이용해 변수등록, 형상드로잉, 재질설정, 여자조건설정, 경계조건설정 등을 할 수 있음

# 모델링을 시작
# 모델링 방법은 제각각이지만, 여기서는 변수기반 모델링을 하기 위해 변수등록을 함
# Step 1. 변수 등록
# 변수등록은 createExpression(key, expression, comment) 메서드를 활용함
modeler.createExpression("N_poles", "8", "Number of Poles")
modeler.createExpression("N_slots", "12", "Number of Slot")

modeler.createExpression("R_so", "100/2", "Stator Outer Diameter")
modeler.createExpression("R_si", "55/2", "Stator Inner Diameter")
modeler.createExpression("R_ro", "54/2", "Rotor Outer Diameter")
modeler.createExpression("R_shaft", "30/2", "Shaft Diameter")
modeler.createExpression("g", "(R_si - R_ro)", "Airgap")

modeler.createExpression("d1", "1", "Shoe Tip1")
modeler.createExpression("d2", "1", "Shoe Tip2")
modeler.createExpression("d3", "15", "Active Slot Depth")
modeler.createExpression("ds", "d1 + d2 + d3", "Slot Depth")
modeler.createExpression("W_t", "8", "Teeth Width")
modeler.createExpression("W_sy", "(R_so-R_si) - ds", "Stator Back Yoke Width")
modeler.createExpression("W_so", "2", "Slot Open Width")

modeler.createExpression("T_m", "2", "Magnet Thickness")
modeler.createExpression("Alpha_m", "0.7", "Pole Arc Ratio (0~1)")
modeler.createExpression("W_ry", "(R_ro-R_shaft) - T_m", "Rotor Back Yoke Width")

modeler.createExpression("R_si1", "R_si + d1", "Stator Inner Radius + Shoe1")
modeler.createExpression("R_si2", "R_si + d1 + d2", "Stator Inner Radius + Shoe1+2")
modeler.createExpression("Theta_so1", "asin((W_so)/(2*R_si1))", "Angle for Half Slot Open @ R_si1")
modeler.createExpression("Theta_shoe1", "_pi/N_slots - Theta_so1", "Angle for Half Shoe @ R_si1")
modeler.createExpression("Theta_tooth2", "asin((W_t)/(2*R_si2))", "Angle for Half Tooth Width @ R_si2")

modeler.createExpression("N_c", "10", "Number of Turns per Coil")
modeler.createExpression("a", "1", "Number of Parallel Branch of the Winding")
modeler.createExpression("RotatingSpeed", "3600", "Rotating Speed in RPM")
modeler.createExpression("f_e", "RotatingSpeed/120*N_poles", "Electric Frequency")
modeler.createExpression("omega", "2*_pi*f_e", "Electric Angular Speed")
modeler.createExpression("I_a", "0", "Amplitude of Phase Current [A_peak]")
modeler.createExpression("beta", "0", "beta Angle [degE]")

# Step 2. 재질등록
# 재질등록을 위해서는 createMaterial(Name, isNonlinear) 메서드를 사용하고
# 생성된 재질의 프로퍼티를 적절히 설정해 사용할 수 있음
# 재질(코어)-->비선형재질
mat_core = modeler.createMaterial("35PN270", True)

# BH커브 데이터 H, B 형태의 셋트로 아래와 같이 표현되어야 함
dataset = '''
0.00E+00,0.00E+00
6.15E+00,5.00E-02
1.23E+01,1.00E-01
1.84E+01,1.50E-01
2.46E+01,2.00E-01
3.07E+01,2.50E-01
3.69E+01,3.00E-01
4.30E+01,3.50E-01
4.92E+01,4.00E-01
5.53E+01,4.50E-01
6.15E+01,5.00E-01
6.77E+01,5.50E-01
7.38E+01,6.00E-01
8.00E+01,6.50E-01
8.62E+01,7.00E-01
9.25E+01,7.50E-01
9.87E+01,8.00E-01
1.05E+02,8.50E-01
1.12E+02,9.00E-01
1.18E+02,9.50E-01
1.25E+02,1.00E+00
1.32E+02,1.05E+00
1.40E+02,1.10E+00
1.48E+02,1.15E+00
1.59E+02,1.20E+00
1.71E+02,1.25E+00
1.89E+02,1.30E+00
2.14E+02,1.35E+00
2.52E+02,1.40E+00
3.15E+02,1.45E+00
4.22E+02,1.50E+00
6.09E+02,1.55E+00
9.47E+02,1.60E+00
1.57E+03,1.65E+00
2.74E+03,1.70E+00
4.99E+03,1.75E+00
9.36E+03,1.80E+00
1.80E+04,1.85E+00
3.55E+04,1.90E+00
7.13E+04,1.95E+00
1.46E+05,2.00E+00
3.04E+05,2.05E+00
6.44E+05,2.10E+00
1.39E+06,2.15E+00
3.04E+06,2.20E+00
6.78E+06,2.25E+00
1.54E+07,2.30E+00
3.55E+07,2.35E+00
8.33E+07,2.40E+00
'''

# 비선형 재질의 경우 BH커브를 등록해 주어야 하는데
# 생성된 재질에서 DataSetNode를 가져와서 여기에 설정해 주어야 함
dataset_node = mat_core.getDataSetNode()
dataset_node.setDataSet(dataset)

# 재질(권선) --> 선형재질
mat_copper = modeler.createMaterial("copper", False)
mat_copper.setConductivity("5.08*1e7")
mat_copper.setPermeability("0.95")

# 재질(영구자석)
# 자화방향 설정이 조금 까다로운 편인데, 사실 아직 문제가 있을 수 있음
# 지금은 Radial착자라 가정했음
mat_magnet_n = modeler.createMaterial("MagnetN", False)

# Radial 착자 설정을 위해 다음과 같이 cos, atan2 함수를 활용함(원통좌표계로 변환하는 거랑 같음)
# 임의 좌표 포인트를 위해 $X,$Y 변수를 사용할 수 있음
# 프로퍼티 설정을 위해 다음과 같은 메서드도 사용할 수 있음
# setPermeability, setMagnetization, setDirectionOfMagnetization
mat_magnet_n.setDirectionOfMagnetization("cos(atan2($Y,$X)), sin(atan2($Y,$X))")
mat_magnet_n.setPermeability("1.05")
mat_magnet_n.setMagnetization("1.25")

mat_magnet_s = modeler.createMaterial("MagnetS", False)
mat_magnet_s.setDirectionOfMagnetization("cos(atan2($Y,$X)), sin(atan2($Y,$X))")
mat_magnet_s.setPermeability("1.05")
mat_magnet_s.setMagnetization("-1.25")

# Step 3. 좌표계 생성
# 좌표계 생성을 위해서는 createCS(origin, angle, name, ref_cs) 를 사용할 수 있음
# ref_cs는 생성할 좌표계가 참조하는 좌표계임
# 글로벌 좌표계에 대한 상대 좌표계를 만들 수 있음 Face좌표계는 지원 안함
default_cs = modeler.getDefaultCS() # 글로벌 좌표계(디폴트 좌표계)를 얻음

cs_shoew_cut = modeler.createCS("R_si1 * cos(Theta_shoe1), R_si1 * sin(Theta_shoe1)", "0", "CS_ShoeCut", default_cs)
cs_half_slotpitch_p = modeler.createCS("0,0", "_pi/N_slots", "CS_HalfSlotPitch_Positive", default_cs)
cs_half_slotpitch_n = modeler.createCS("0,0", "-_pi/N_slots", "CS_HalfSlotPitch_Negative", default_cs)
cs_cut_shoe_p = modeler.createCS("0,-W_so/2", "0", "CS_CutShoeP", cs_half_slotpitch_p)
cs_cut_shoe_n = modeler.createCS("0,W_so/2", "0", "CS_CutShoeN", cs_half_slotpitch_n)
cs_cut_slotL_base = modeler.createCS("0,0", "2*_pi/N_slots", "CS_CutSlotLBase", default_cs)
cs_cut_slotL = modeler.createCS("0,-W_t/2", "0", "CS_CutSlotL", cs_cut_slotL_base)
cs_cut_slotR = modeler.createCS("0,W_t/2", "0", "CS_CutSlotR", default_cs)
cs_cut_magnet = modeler.createCS("0,0", "2*_pi/N_poles * Alpha_m", "CS_CutMagnet", default_cs)
cs_cut_model = modeler.createCS("0,0", "2*_pi/4", "CS_CutModel", default_cs)


# Step 4. 형상 모델링
# Stetp 4.1 고정자 백요크, 외곽 에어더미
# 기초형상을 만드는 메서드는 네가지가 다섯가지가 있다
# 1) 원만들기: createCircle(origin, radius, num_segments, name)
#    orign -> 원점으로 "x,y" 형태로 표현해야 함 (데이터 타입: 문자열)
#    radius -> 반지름 (데이터 타입: 문자열)
#    num_segments -> 다각형으로 만들고 싶을때 사용, 3미만이면 다각형을 생성할 수 없으므로 적용안됨 (데이터 타입: 문자열)
#    name -> 객체 이름 (데이터 타입: 문자열)
# 2) 직사각형: createRectangle(origin, dx, dy, name)
# 3) 직선: createLine(strat, end, name)
#    start -> 시작점으로 "x,y" 형태로 표현해야 함 (데이터 타입: 문자열)
#    end   -> 끝점으로 "x,y" 형태로 표현해야 함
# 4) 아크(호): createCurve(start, end, origin, radius, name)
#    호를 지탱하는 원의 정보를 주어야 하고, 원주상에서 시작점과 끝점을 주는 형태로 생성함
#    시작점과 끝점이 정확하게 원주상에 존재하지 않게되면 원주상에 가장 가까운 점으로 대체함
# 5) 복제: 기존 만들어진 형상을 복제하는 함수: clone(ref_node, name)
stator_ext = modeler.createCircle("0,0", "R_so * 1.25", "0", "StatorExtend")
stator = modeler.createCircle("0,0", "R_so", "0", "Stator")
stator_bore = modeler.createCircle("0,0", "R_so - W_sy", "0", "Stator_bore")
stator_inner = modeler.createCircle("0,0", "R_si", "0", "StatorInner")

# 하기 코드에서 복제를 하는데 위에서 만들어진 stator를 복제하는 것임
# stator는 단순 원임을 인지하기 바람; 아래 코드에 빼기코드가 존재하나
# 코드가 순차적으로 실행됨에 따라 여기에서의 stator는 그냥 원임
# 즉, 그냥 원을 복제하는 것임
air_stator = modeler.clone(stator, "AirStator")
stator_ext_in = modeler.clone(stator, "StatorExtend_in")

shoe_inner = modeler.clone(stator_inner, "ShoeInner")
airstator_in = modeler.clone(stator_inner, "AirStatorInner")

stator_ext = modeler.subtract(stator_ext, stator_ext_in)
stator = modeler.subtract(stator, stator_bore)
air_stator = modeler.subtract(air_stator, airstator_in);

# Stetp 4.2 고정자 치
tooth = modeler.createRectangle("0, -W_t/2", "R_so-W_sy/2", "W_t", "Tooth")

shoe2 = modeler.createCircle("0,0", "R_si+d1+d2", "0", "ShoeTop")
shoe1 = modeler.createCircle("0,0", "R_si+d1", "0", "ShoeMiddle")
clone_shoe1 = modeler.clone(shoe1, "ShoeMiddle_1")
shoe2 = modeler.subtract(shoe2, clone_shoe1)
shoe1 = modeler.subtract(shoe1, shoe_inner)
tooth = modeler.subtract(tooth, stator_inner)

# 처음으로 split 함수가 나옴
# 좌표계를 기준으로 zx평면 혹은 yz 평면으로 자르는 것임
# split(ref_node, plane, selected_plane, ref_cs)
# ref_node -> 자를 형상
# plane -> 자르는 평면, 0: zx, 1: yz 다른 숫자는 쓰면 안됨
# selected_plane -> 자른후 남기는 평면(오른손 법칙에 따름; True일 경우 엄지손가락 방향, Fales경우 그 반대방향)
# ref_cs -> 참조 좌표계
shoe1 = modeler.split(shoe1, 0, True, cs_cut_shoe_n)
shoe1 = modeler.split(shoe1, 0, False, cs_cut_shoe_p)

shoe2 = modeler.split(shoe2, 0, True, cs_cut_shoe_n)
shoe2 = modeler.split(shoe2, 0, False, cs_cut_shoe_p)

tooth = modeler.unite(tooth, shoe1)
tooth = modeler.unite(tooth, shoe2)

# 치를 복제
for i in range(1, 4):
    # 이름 생성
    name = "Tooth_" + str(i)

    # 복제 수행
    clone_tooth = modeler.clone(tooth, name)

    # 각도 계산
    angle = "2*_pi/N_slots * " + str(i)

    # 회전 수행
    clone_tooth = modeler.rotate(clone_tooth, angle)

    # 병합 수행
    modeler.unite(stator, clone_tooth)

# 최종 병합
stator = modeler.unite(stator, tooth)

# 고정자 재질 설정(주의!)
# 재질은 만들어진 형상의 해더에 적용을 해야 함; 프로그램의 트리를 확인해 보라
# getHeadNode() 메서드를 이용해 stator의 해더를 구하고, 거기에 재질을 설정함
stator.getHeadNode().setMaterialNode(mat_core)

# Stetp 4.3 슬롯
slotL = modeler.createCircle("0,0", "R_si+d1+d2+d3", "0", "SlotL_0")
slotL_in = modeler.createCircle("0,0", "R_si+d1+d2+2", "0", "Slot_in")
slotL = modeler.subtract(slotL, slotL_in)
slotL = modeler.split(slotL, 0, True, cs_cut_slotR)
slotL = modeler.split(slotL, 0, False, cs_cut_slotL)
slotR = modeler.clone(slotL, "SlotR_0")
slotL = modeler.split(slotL, 0, True, cs_half_slotpitch_p)
slotR = modeler.split(slotR, 0, False, cs_half_slotpitch_p)

# 슬롯 재질 설정(주의!)
slotL.getHeadNode().setMaterialNode(mat_copper)
slotR.getHeadNode().setMaterialNode(mat_copper)

# 추가 팁 (색상설정)
# 색상또한 헤더에 설정함
# R, G, B, Alpha(불투명도) 0~255값, (255, 0, 0, 255)이면 빨간색 불투명도 최대가 됨
slotL.getHeadNode().setColor(255, 170, 0, 255);
slotR.getHeadNode().setColor(255, 170, 0, 255);

# 슬롯 복제
# 복제된 슬롯을 담을 변수
slotLs = []
slotRs = []

# 초기 슬롯을 리스트에 추가
slotLs.append(slotL)
slotRs.append(slotR)

for i in range(1, 3):
    # 이름 생성
    name1 = f"SlotL_{i}"
    name2 = f"SlotR_{i}"

    # 슬롯 복제
    clone_slotL = modeler.clone(slotL, name1)
    clone_slotR = modeler.clone(slotR, name2)

    # 각도 계산 및 회전
    angle = f"2*_pi/N_slots*{i}"
    clone_slotL = modeler.rotate(clone_slotL, angle)
    clone_slotR = modeler.rotate(clone_slotR, angle)

    # 복제된 노드 색상 및 재질 설정
    clone_slotL.getHeadNode().setColor(255, 170, 0, 255)
    clone_slotR.getHeadNode().setColor(255, 170, 0, 255)
    clone_slotL.getHeadNode().setMaterialNode(mat_copper)
    clone_slotR.getHeadNode().setMaterialNode(mat_copper)

    # 슬롯 리스트에 추가
    slotLs.append(clone_slotL)
    slotRs.append(clone_slotR)
    
# Stetp 4.4 공극
airgap = modeler.createCircle("0,0", "R_si", "0", "Airgap")
airgap_in = modeler.createCircle("0,0", "R_ro", "0", "AirgapIn")
airgap = modeler.subtract(airgap, airgap_in)

# Stetp 4.5 회전자
magnet = modeler.createCircle("0,0", "R_ro", "0", "Magnet_N")
air_rotor = modeler.clone(magnet, "AirRotor")
rotor = modeler.createCircle("0,0", "R_ro-T_m", "0", "Rotor")

clone_magnet_in = modeler.clone(rotor, "Magnet_in_1")
magnet = modeler.subtract(magnet, clone_magnet_in)
magnet = modeler.split(magnet, 0, True, default_cs)
magnet = modeler.split(magnet, 0, False, cs_cut_magnet)
magnet = modeler.rotate(magnet, "_pi/N_poles * (1-Alpha_m)")

magnet_2 = modeler.clone(magnet, "Magnet_S")
magnet_2 = modeler.rotate(magnet_2, "2*_pi/N_poles")

shaft = modeler.createCircle("0,0", "R_shaft", "0", "Shaft")

# 회전자 재질설정,
rotor.getHeadNode().setMaterialNode(mat_core)

# 영구자석 재질설정
magnet.getHeadNode().setMaterialNode(mat_magnet_n)
magnet_2.getHeadNode().setMaterialNode(mat_magnet_s)

# 영구자석 칼라설정
magnet.getHeadNode().setColor(255, 0, 0, 255)
magnet_2.getHeadNode().setColor(0, 0, 255, 255)


# Stetp 4.6 모델 분할
# 초기 객체 리스트 생성
spilt_objs = [stator, air_stator, air_rotor, rotor, stator_ext, airgap, shaft]

# 객체 분할 처리
for obj in spilt_objs:
    temp = modeler.split(obj, 0, True, default_cs)
    modeler.split(temp, 0, False, cs_cut_model)
    
# Stetp 4.7 경계 생성
ext_boundary = modeler.createCurve("R_so * 1.25, 0", "0, R_so * 1.25", "0,0", "R_so * 1.25", "StatorExt")
master = modeler.createLine("0,0", "R_so * 1.25,0", "Master")
slave = modeler.clone(master, "Slave")
slave = modeler.rotate(slave, "2*_pi/4")


# Step 5. 경계조건 생성
# 경계조건은 맥스웰과는 조금 달리 오브젝트 자체를 경계조건에 할당하는 방식이다
# 오브젝트의 라인을 찍어서 경계조건에 할당할수는 없다
# 방법은 우선 경계조건을 만든다
# 경계조건의 종류는, 고정경계, 주기경계(마스터, 슬레이브), 무빙경계가 있다
# 하기는 고정경계를 만드는 것임
bc = modeler.createDirichletBC("0", "ZeroPotential")

# 고정경계에 해당 오브젝트(헤더)를 설정
modeler.createBCObject(ext_boundary.getHeadNode(), "Fixed1", bc)

# 주기경계조건(마스터)
master_bc = modeler.createMasterBC(True, "Master")

# 주기경계에 해당하는 헤더를 설정
modeler.createBCObject(master.getHeadNode(), "Master", master_bc)

# 주기경계조건(슬레이브)
slave_bc = modeler.createSlaveBC(master_bc, True, True, "Slave")

# 주기경계(슬레이브)에 해당하는 헤더를 설정
modeler.createBCObject(slave.getHeadNode(), "Slave", slave_bc)

# 무빙경계(무빙밴드) 설정, 회전속도와, 초기위치를 인자로 받음
mb = modeler.createMovingBand("RotatingSpeed", "0", "MovingBand")
modeler.createBCObject(airgap.getHeadNode(), "Band", mb)

# Step 6. 여자조건 생성
# 와인딩을 생성; 맥스웰과 비슷하게 권선에 여자조건을 줌
phaseA = modeler.createWinding("I_a*cos(omega*$TIME + beta)", "1", "PhaseA")
phaseB = modeler.createWinding("I_a*cos(omega*$TIME + beta - 2*_pi/3)", "1", "PhaseB")
phaseC = modeler.createWinding("I_a*cos(omega*$TIME + beta + 2*_pi/3)", "1", "PhaseC")

# 권선에 포함되는 코일을 생성; 코일의 턴수와 전류의 방샹을 인자로 줌
phaseA_coil = modeler.createCoil("N_c", True, "PhaseA+", phaseA)
phaseA_recoil = modeler.createCoil("N_c", False, "PhaseA-", phaseA)
phaseB_coil = modeler.createCoil("N_c", True, "PhaseB+", phaseB)
phaseB_recoil = modeler.createCoil("N_c", False, "PhaseB-", phaseB)
phaseC_coil = modeler.createCoil("N_c", True, "PhaseC+", phaseC)
phaseC_recoil = modeler.createCoil("N_c", False, "PhaseC-", phaseC)

# 코일의 레퍼런스 노드 설정
# 실제 코일이 어떤 오브젝트와 연관되는지 설정해 주어야 함
phaseA_coil.setReferenceNode(slotLs[2].getHeadNode())
phaseA_recoil.setReferenceNode(slotRs[0].getHeadNode())
phaseB_coil.setReferenceNode(slotLs[0].getHeadNode())
phaseB_recoil.setReferenceNode(slotRs[1].getHeadNode())
phaseC_coil.setReferenceNode(slotLs[1].getHeadNode())
phaseC_recoil.setReferenceNode(slotRs[2].getHeadNode())

# Step 7. 트랜지언트 해석 조건 생성
setup = modeler.getDefaultSetup()
transient = modeler.createTransientSetup("Transient", setup)
transient.setStopTime("1/f_e")
transient.setTimeStep("1/f_e/90")
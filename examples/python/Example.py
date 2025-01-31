# Python Example for bzmagEdior

# imort bzmagPy
import bzmagPy as bzmag

# get Modeler
modeler = bzmag.get('/sys/modeler')


# Register Expressions
modeler.createExpression("Core_Width", "25", "Transformer Core Width (based on Outbounded)")
modeler.createExpression("Core_Height", "20", "Transformer Core Height (based on Outbounded)")
modeler.createExpression("Path_Width", "5", "Transformer Core Band Width")
modeler.createExpression("Window_Width", "Core_Width-2*Path_Width", "Window Width")
modeler.createExpression("Window_Height", "Core_Height-2*Path_Width", "Window Height")
modeler.createExpression("WiningWidth", "5", "Winding Width")
modeler.createExpression("AirgapWidth", "1", "Winding Width")
modeler.createExpression("AirModel_Width", "Core_Width+2*WiningWidth", "Airmodel Width")
modeler.createExpression("N_c1", "10", "Number of Turns for the Primary Winding")
modeler.createExpression("N_c2", "20", "Number of Turns for the Secondary Winding")
modeler.createExpression("I_a", "20", "Applied Current of Primary Winding")

# 코어
core = modeler.createRectangle("-Core_Width/2, -Core_Height/2", "Core_Width", "Core_Height", "Core")
core_in = modeler.createRectangle("-Window_Width/2, -Window_Height/2", "Window_Width", "Window_Height", "Core_in")
airgap = modeler.createRectangle("-AirgapWidth/2, 0", "AirgapWidth", "Core_Height", "Airgap")
modeler.subtract(core, core_in)
modeler.subtract(core, airgap)

# 1차측 권선
winding11 = modeler.createRectangle("-Core_Width/2, -Window_Height/2", "-WiningWidth", "Window_Height", "PrimaryWining(Left)").getHeadNode()
winding12 = modeler.createRectangle("-Window_Width/2, -Window_Height/2", "WiningWidth", "Window_Height", "PrimaryWining(Right)").getHeadNode()
winding11.setColor(255, 170, 0, 230)
winding12.setColor(255, 170, 127, 230)

# 2차측 권선
winding22 = modeler.createRectangle("Core_Width/2, -Window_Height/2", "WiningWidth", "Window_Height", "SecondaryWining(Right)").getHeadNode()
winding21 = modeler.createRectangle("Window_Width/2, -Window_Height/2", "-WiningWidth", "Window_Height", "SecondaryWining(Left)").getHeadNode()
winding21.setColor(170, 170, 0, 230)
winding22.setColor(170, 170, 127, 230)

# AirModel
airmodel = modeler.createRectangle("-AirModel_Width/2 * 1.2, -Core_Height/2 * 1.2", "AirModel_Width * 1.2", "Core_Height * 1.2", "AirModel").getHeadNode()
airmodel.setColor(255, 255, 255, 0)

# 경계라인 (반시계 방향)
bc1 = modeler.createLine("-AirModel_Width/2 * 1.2, -Core_Height/2 * 1.2", "AirModel_Width/2 * 1.2, -Core_Height/2 * 1.2", "Boundary_Bottom").getHeadNode()
bc2 = modeler.createLine("AirModel_Width/2 * 1.2, -Core_Height/2 * 1.2", "AirModel_Width/2 * 1.2, Core_Height/2 * 1.2", "Boundary_Right").getHeadNode()
bc3 = modeler.createLine("AirModel_Width/2 * 1.2, Core_Height/2 * 1.2", "-AirModel_Width/2 * 1.2, Core_Height/2 * 1.2", "Boundary_Top").getHeadNode()
bc4 = modeler.createLine("-AirModel_Width/2 * 1.2, Core_Height/2 * 1.2", "-AirModel_Width/2 * 1.2, -Core_Height/2 * 1.2", "Boundary_Left").getHeadNode()

# 경계조건 생성
bc = modeler.createDirichletBC("0", "ZeroPotential")
ref_head1 = modeler.createBCObject(bc1, "Fixed1", bc)
ref_head2 = modeler.createBCObject(bc2, "Fixed2", bc)
ref_head3 = modeler.createBCObject(bc3, "Fixed3", bc)
ref_head4 = modeler.createBCObject(bc4, "Fixed4", bc)

# 재질(코어)
mat_core = modeler.createMaterial("35PN270", True)
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
dataset_node = mat_core.getDataSetNode()
dataset_node.setDataSet(dataset)
mat_core.Permeability = "1000"

mat_copper = modeler.createMaterial("copper", False)
mat_copper.Conductivity = "5.08*1e7"
mat_copper.Permeability = "0.95"

# 코어재질 설정
core_head = core.getHeadNode()
core_head.setMaterialNode(mat_core)

# 권선재질 설정
winding11.setMaterialNode(mat_copper)
winding12.setMaterialNode(mat_copper)
winding21.setMaterialNode(mat_copper)
winding22.setMaterialNode(mat_copper)

# 여자조건 생성
primary = modeler.createWinding("I_a", "1", "PrimaryWinding")
secondary = modeler.createWinding("0", "1", "SecondaryWinding")
prim_coil = modeler.createCoil("N_c1", True, "Primary+", primary)
prim_recoil = modeler.createCoil("N_c1", False, "Primary-", primary)
second_coil = modeler.createCoil("N_c2", True, "Secondary+", secondary)
second_recoil = modeler.createCoil("N_c2", False, "Secondary-", secondary)

prim_coil.setReferenceNode(winding11)
prim_recoil.setReferenceNode(winding12)
second_coil.setReferenceNode(winding21)
second_recoil.setReferenceNode(winding22)

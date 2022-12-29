# CAD

All 3D printed parts are printed with PLA using a layer height of 0.15 or 0.2. It will be necessary to print all with adhesion and without support.

## Motor base holder

Layer height 0.15 mm. Infill 60%

- FreeCAD source file: [./freecad/motor_base_holder.FCStd](./freecad/motor_base_holder.FCStd)
- STEP file: [./step/motor_base_holder.step](./step/motor_base_holder.step)
- STL file to print: [./stl/motor_base_holder.stl](./stl/motor_base_holder.stl)
- Python file to generate FreeCAD file (you need [Maker Workbench](https://github.com/URJCMakerGroup/MakerWorkbench)) : [./py/motor_base_holder.py](./py/motor_base_holder.py)



## Geared NEMA-17 motor holder

Layer height 0.2 mm. Infill 60%

- FreeCAD source file: [./freecad/motor_holder.FCStd](./freecad/motor_holder.FCStd)
- STEP file: [./step/motor_holder.step](./step/motor_holder.step)
- STL file to print: [./stl/motor_holder.stl](./stl/motor_holder.stl)
- Python file to generate FreeCAD file (you need [Maker Workbench](https://github.com/URJCMakerGroup/MakerWorkbench)) : [./py/NemaMotorHolder_class.py](./py/NemaMotorHolder_class.py)


## Joint for Leadscrew Nut - Gantry

This part joints the leadscrew nut with the base of the gantry

Layer height 0.2 mm. Infill 60%

- FreeCAD source file: [./freecad/nut_gantry_joint.FCStd](./freecad/nut_gantry_joint.FCStd)
- STEP file: [./step/nut_gantry_joint.step](./step/nut_gantry_joint.step)
- STL file to print: [./stl/nut_gantry_joint.stl](./stl/nut_gantry_joint.stl)
- Python file to generate FreeCAD file (you need [Maker Workbench](https://github.com/URJCMakerGroup/MakerWorkbench)) : [./py/nut_gantry_joint.py](./py/nut_gantry_joint.py)


## Linear bearing housing

Four pieces are needed for the top and bottom, the top part is 1 mm (21 mm) taller than the bottom part

Layer height 0.15 mm. Infill 60%

- FreeCAD source file: [./freecad/linbearhouse.FCStd](./freecad/linbearhouse.FCStd)
- STEP file: [./step/linbearhouse.step](./step/linbearhouse.step)
- STL file to print the top part: [./stl/linbearhouse_top.stl](./stl/linbearhouse_top.stl)
- STL file to print the bottom part: [./stl/linbearhouse_bot.stl](./stl/linbearhouse_bot.stl) 
- The [Maker Workbench](https://github.com/URJCMakerGroup/MakerWorkbench) can generate configurable linear bearing housings


## Endstop holder

Two pieces are needed (the design needs improvement)

Layer height 0.2 mm. Infill 60%

- FreeCAD source file: [./freecad/endstop_holder.FCStd](./freecad/endstop_holder.FCStd)
- STEP file: [./step/endstop_holder.step](./step/endstop_holder.step)
- STL file to print: [./stl/endstop_holder.stl](./stl/endstop_holder.stl)

## Arduino base support

Layer height 0.2 mm. Infill 40%

- STL file to print: [./stl/arduino_cover.stl](./stl/arduino_cover.stl)

## LCD support

These are taken from [Prusa Hefestos](https://github.com/bq/prusa-i3-hephestos)

There is a holder and a hinge:

- FreeCAD source files:
    - [./freecad/lcd-holder.FCStd](./freecad/lcd-holder.FCStd)
    - [./freecad/lcd-hinge.FCStd](./freecad/lcd-hinge.FCStd)
- STEP source files:
    - [./step/lcd-holder.step](./step/lcd-holder.step)
    - [./step/lcd-hinge.step](./step/lcd-hinge.step)
- STL source files:
    - [./stl/lcd-holder.stl](./stl/lcd-holder.stl)
    - [./stl/lcd-hinge.stl](./stl/lcd-hinge.stl)


## Optical sensor holder (Optional)

This is only if you include the optical sensor

Layer height 0.2 mm. Infill 40%

- FreeCAD source file: [./freecad/optendstopical_sensor_holder.FCStd](./freecad/optendstopical_sensor_holder.FCStd)
- STEP file: [./step/optendstopical_sensor_holder.step](./step/optendstopical_sensor_holder.step)
- STL file to print: [./stl/optendstopical_sensor_holder.stl](./stl/optendstopical_sensor_holder.stl)


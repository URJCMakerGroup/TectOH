# Tectonic Open Hardware (TectOH) Sandbox


![Sandbox CAD image](./imgs/cad_lateral_view_sm.png)

Low-cost Open-Hardware scientific and educational device for tectonic analogue modelling. See Wikipedia entry on [tectonic analogue modeling](https://en.wikipedia.org/wiki/Analogue_modelling_%28geology%29).

It can move at speeds from 1 mm/h to 100 mm/h.

The stroke of the gantry is 270 mm but it can be easily customized for other lengths.

The cost of the components is no more than 500 €, although for increased performance, precision components can be used at higher cost.

There are some [optional components](#distance-monitoring) for **measuring the distance** that would increase the cost.

It is not too complicated to build. It should be simpler than building a 3D printer or CNC.


This is the second version of the Open Hardware Sandbox. [Link to the first version](https://github.com/URJCMakerGroup/TFG-Cristina-Fernandez)

----

## License

Hardware License: [CERN-OHL-S 2.0](cern_ohl_s_v2.txt)

Software License: [LGPL 3.0 or later](License.md)

Documentation License: [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)

![license summary](imgs/oshw_lic.png)

![ES000033](imgs/oshwa_es000033.png)

This device is [certified open hardware](https://certification.oshwa.org/es000033.html) by the [Open Source Hardware Association](https://www.oshwa.org/).

----

## Bill of materials

It is about 500€ without taxes and shipping costs.

There could be cheaper alternatives in your area.

You can use more expensive mechanical components to increase the overall precision of the system

[Sandbox bill of materials (.ods file)](tectoh_bom.ods)

----

## Assembly

[Sandbox assembly](./assembly/.)

----

## Firmware

[Arduino code of the Sandbox](./firmware/.)

----

## Distance monitoring

[Optical or magnetic linear sensor](./optional/.) can be included in the Sandbox to monitor the displacement.
These are optional subsystems, but they are very helpful to calibrate the Sandbox.

This section includes graphs of the **results** and Python scripts to analyze the data from the magnetic sensor.


----

## User guide

[Sandbox user guide](./userguide/.)

----

## Pictures

Sandbox experiment

![Experiment](./imgs/foto_box.jpg)


The Sandbox pushing a 10-kg sandpile

![Picture 10-kg sandpile](./imgs/foto_sandpile.jpg)

The Sandbox pushing a 5-kg sand load constrained within a box

![Picture 5-kg box](./imgs/foto_5kg_box.jpg)

----

## Contributors

This work has been developed at [Universidad Rey Juan Carlos](https://www.urjc.es/) in Spain. It has been a collaboration between the Area of Electronic Technology and the group [Tecvolrisk](https://tecvolrisk.wixsite.com/website) of the Area of Geology.

### Area of Electronic Technology:

- [Felipe Machado](https://github.com/felipe-m/)
- [Rubén Nieto](https://gestion2.urjc.es/pdi/ver/ruben.nieto)
- [Susana Borromeo](https://gestion2.urjc.es/pdi/ver/susana.borromeo)

### Tecvolrisk (Area of Geology)

- [Marta Rincón](https://gestion2.urjc.es/pdi/ver/marta.rincon)
- [Sandra González-Muñoz](https://tecvolrisk.wixsite.com/website/sandra-gonz%C3%A1lez-mu%C3%B1oz)
- [Fidel Martín-González](https://gestion2.urjc.es/pdi/ver/fidel.martin)

### Students

Some students have collaborated in the project doing their capstone project in Industrial Engineering.

- [Adrián Zeus Román](https://github.com/zeus97roman/tfg): Initial project
- [Cristina Fernández García](https://github.com/cfg97/TFG-Cristina-Fernandez): Finished first version
- [David Muñoz Bernal](https://github.com/davidmubernal/MakerWorkbench): FreeCAD workbench that includes many CAD parts of the project 
- [Javier Letón](https://github.com/jleton10/TFG_Javier_Leton): Added the linear optical sensor
- [Alicia Merchán](https://github.com/AliciaMH/TFGAliciaMerchan): future work in adding a second degree of freedom






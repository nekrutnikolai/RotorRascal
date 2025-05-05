For the build guide, documentation, and more, please check out our [Wiki page](https://github.com/cornellmotionstudio/JacksonDronev2/wiki).

<p align="center">
    <img width="600" alt="Drone" src="figs/drone.jpeg">
</p>

# Getting Started

Clone the project
```
git clone https://github.com/nekrutnikolai/RotorRascal.git
```

Go to the software dir.
```
cd RotorRascal && cd software
```

Build the project

```
./builder
```

## Final Report Components

### Project Introduction
- One sentence "sound bite" that describes your project.
    We built and implemented PID control on a Pico-powered quadcopter to hover on a gimbal stand with throttle input from a controller. 

- A summary of what you did and why.
    

### High level design

- Rationale and sources of your project idea
    Bryan, Martin and I enjoy drones and quadcopters, we thought it would be a neat idea to develop a somewhat functioning flight controller to control the drone with an onboard IMU for orientation data, an external remote to set the throttle position, and an electronic speed controller (ESC). 
    
    Nikolai is a part of the Motion Studio "Drone Squad" advised by Dr. Beatriz Asfora which was involved in the creating of this final project idea. Out of the Drone Squad there was already a prototype developed by Deemo Chen with a PCB integrating an IMU and Pi Pico along with a skeleton of code on how to control each PWM ESC channel.

    The long-term objective of this project is to support the integration of these Pico-based drones made from 3D printed components into ECE and MAE courses. For instance, these drones could be integrated in the ECE microcontrollers curriculum and the students could work on code developing/tuning a PID controller. Or, for a MAE dynamics course, the students could use these drones for dynamics modelling.

- background math
    

- logical structure
    1. Initialize radio and sensors
    2. Calibrate ESCs and Gyro
    3. Initialize serial PID tuning interface
    4. Wait for arm signal from controller
        1. Erase all store PID values -- effectively reset controls
    5. Spin up motors to minimum throttle and begin executing PID loop
    6. Continue executing PID loop until disarm

<p align="center">
    <img width="600" alt="Drone" src="figs/DroneFlowChart.jpeg">
</p>

- hardware/software tradeoffs
    **PID loop speed:** Since the drone is a complicated, unstable system with 4 motors and 6 degrees of freedom, it is vital to have a fast control loop. From our testing and with lab 3, we found a value of 1kHz to work well. Compared to lab 3, we need to be computing the PID terms for 3 axes as opposed to a single PID axis. 

    **Pico W:** The Pico W was used to allow future expansion of wireless capabilites. Future work could involve developing a web server that hosts an interactive PID tuning interface where students in courses could drag sliders and 

    **Controlling 4 Motors:** The PWM needs to be computed for each of the 4 motors, which takes more time to calculate.

    **Yaw Compensation:**

    **Long-Term Gyro Drift:** calibrating gyro upon power-up. 

- Discuss existing patents, copyrights, and trademarks which are relevant to your project.
    There are no existing patents, copyrights, and trademarks which are relevant to our project.

### Program/hardware design
- program details. What parts were tricky to write?

- hardware details. Could someone else build this based on what you have written?
    - Make this into a build guide!
- Be sure to specifically reference any design or code you used from someone else.
    - Add sources from the drone squad. 
- Things you tried which did not work

### Results of the design
- Any and all test data, scope traces, waveforms, etc
- speed of execution (hesitation, filcker, interactiveness, concurrency)
- accuracy (numeric, music frequencies, video signal timing, etc)
- how you enforced safety in the design.
- usability by you and other people

### Conclusions
- Analyse your design in terms of how the results met your expectations. What might you do differently next time?
    Got it working pretty well but it seems almost that we could have gotten it working better perhaps?
- How did your design conform to the applicable standards?
- Intellectual property considerations.
    - Did you reuse code or someone else's design? Did you use any of Altera's IP?
    - Did you use code in the public domain?
    - Are you reverse-engineering a design? How did you deal with patent/trademark issues?
    - Did you have to sign non-disclosure to get a sample part?
    - Are there patent opportunites for your project?

### Appendix A (permissions)
- "The group approves this report for inclusion on the course website."
- "The group approves the video for inclusion on the course youtube channel."

### Additional appendices
- Appendix with commented Verilog and/or program listings.
- Appendix with schematics if you build hardware external to the DE1-SoC board (you can download free software from expresspcb.com to draw schematics).
- Appendix with a list of the specific tasks in the project carried out by each team member.
- References you used:
    - Data sheets
    - Vendor sites
    - Code/designs borrowed from others
    - Background sites/paper 
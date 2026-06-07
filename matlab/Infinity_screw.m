clc, clear all 

%%
%Actuator requirements:
F_max_req = 80;     %Required vertical force on arm [N]
t_req = 1;          %Required actuation time [s] (0-80N)
F_step_req = 0.75;  %Required accuracy [N]

%Screw data
dm = 8*10^-3;  %mean diameter of thread [m]
f = 0.2;        %Coefficient of friction between screw and nut 
p = 2*10^-3;  %pitch [m]
starts = 1;     %Number of starts on the screw thread

%Motor data
T_m = 0.6;        %Maximum motor torque [Nm]
n = 1000;        %Maximum motor rpm 
spr = 200;      %Steps per revolution

%Other 
k = 15*10^3;       %Spring constant [N/m] 


%Calculates the maxmimum vertical force (F_max) based on the maximum motor torque
%(T_m)
l = p*starts; %Lead of thread [m]
F_max = 2*T_m/dm*((pi*dm-f*l)/(l+pi*f*dm));

%Calculates the time it takes for the vertical force to go from 0-80N
%assuming that the motor opperates at max rpm.
v = l*n/60;     %Velocity of nut [m/s]
delta_y = F_max_req/k;  %Required spring displacment [m] to achieve a force of F=80N
t = delta_y/v;  %Time it takes to achieve said force [s]


%Caculates the accuracy of the actuator (we want this to be less than 0.75N):
lin_res = l/spr;    %Linear resultion [m] (the distance the nut travels with each step of the motor)
F_step = k*lin_res; %Force from each step of the motor [N] (Hooke's law)


%Prints
T_req = table(F_max_req, t_req, F_step_req, ...
    'VariableNames',{'F_max_req [N]','t_req [s]', 'F_step_req [N]'});
T_per = table(F_max, t,  F_step,...
    'VariableNames', {'F_max [N]','t [s]','F_step [N]'});
T_per = round(T_per, 3);  % 4 significant digits
T_other = table(v*1e3, delta_y*1e3, lin_res*1e3, ...
    'VariableNames', {'v [mm/s]','delta_y [mm]','lin_res [mm]'});

disp('--- Actuator system requirements ---')
disp(T_req)
disp('--- Actuator Performance ---')
disp(T_per)
disp('--- Other ---')
disp(T_other)


%%
%Calculates the pressure on the disc based on the force that is applied to
%the arm. This force is meaured with an s-shaped sensor.

F_s = 80; %Meassured force [N]
L1 = 0.3; % Length of the arm [m]
L2 = 0.2; % Length of the arm [m]
r = 5*10^-3; % Radius of the pin [m]

p_disc = F_s*(L1+L2)/(pi*(r^2)*L2); 

fprintf('p_disc = %.4g MPa\n', p_disc*1e-6)



close all, clc, clf, clear all;

ser1 = serialport("COM3", 9600);
fopen(ser1);

points = 200;
load_max = 2000;

load_in = zeros(1, points);
% load_in(end) = 1;
timestamps = zeros(1, points);
timestamps(end) = 1;
ref = zeros(1, points);
% ref(end) = 1;
speed = zeros(1, points);
% speed(end) = 1;

fig1 = figure(1);

% subplot(1, 2, 1)
hold on;
grid on;
plt1 = plot(timestamps, load_in);
plt2 = plot(timestamps, ref);
ylim([-load_max, load_max]);

% subplot(1, 2, 2)
% grid on;
% plt3 = plot(timestamps, speed);

time_tr_begin = 0;
time_tr_stop = 0;
switch1 = true;
switch2 = true;
switch3 = true;
Tr = 0;

while(true)
    ser_read = fscanf(ser1);
    reading = str2num(ser_read);

    % disp(reading)

    if(~isnan(reading))
        %% Plot points

        load_in(end + 1) = reading(1);
        load_in(1) = [];

        timestamps(end + 1) = reading(2);
        timestamps(1) = [];

        ref(end + 1) = reading(3);
        ref(1) = [];

        speed(end + 1) = reading(4);
        speed(1) = [];
        
        xlim([timestamps(1), timestamps(end)]);
        set(plt1, 'XData', timestamps, 'YData', load_in);
        set(plt2, 'XData', timestamps, 'YData', ref);

        %% Calculate Tr
        
        if(switch1)
            bkgrd = mean(load_in(end-20:end));
        end
        if(load_in(end) > 1.1*bkgrd)
            switch1 = false;
            time_tr_begin = timestamps(end);
        end
        if(load_in(end) < 0.9*ref(end) && ~switch1 && switch2)
            switch2 = false;
            time_tr_stop = timestamps(end);
            Tr = time_tr_stop - time_tr_begin;
            disp(Tr);
        end
        if(~switch1 && ~switch2 && load_in(end) < bkgrd)
            switch1 = true;
            switch2 = true;
        end

        % bkgrd_points = 10;
        % max_cutoff = 5;
        % latest_points = load_in(end-bkgrd_points:end);
        % if(switch1 && switch2 && switch3)
        %     bkgrd = mean(latest_points);
        % end
        % sens = 1.1;
        % Tr_target = 0.9;
        % 
        % if(load_in(end) > sens*bkgrd && switch1)
        %     switch1 = false;
        %     time_tr_begin = timestamps(end);
        % end
        % if(load_in(end) > Tr_target*ref(end) && ~switch1 && switch2)
        %     switch2 = false;
        %     time_tr_stop = timestamps(end);
        %     Tr = time_tr_stop-time_tr_begin;
        %     disp("Tr: " + Tr/1000)
        % end
        % if(load_in(end) < sens*bkgrd)
        %     switch1 = true;
        %     switch2 = true;
        %     switch3 = true;
        %     time_tr_begin = 0;
        %     time_tr_stop = 0;  
        % end
        % 
        % %% Calculate Ts
        % 
        % % Ts = 1;
        % 
        % %% Calculate M
        % if(max(latest_points) > ref(end) && max(load_in(end-max_cutoff:end) < max(latest_points)) && switch3)
        %     switch3 = false;
        %     M = (max(latest_points) - ref(end))/ref(end);
        %     disp("M: " + M)
        % end
        % 
        % disp(bkgrd)

        % xlim([toplotx(end-points), toplotx(end)]);
        % set(plt3, 'XData', toplotx, 'YData', toplotspeed);
        % xlim([toplotx(end-points), toplotx(end)]);
    end
end

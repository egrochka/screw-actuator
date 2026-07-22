close all, clc, clf, clear all;

ser1 = serialport("COM3", 9600);
fopen(ser1);

points = 200;
load_max = 5000;
speed_max = 9000;

load_in = zeros(1, points);
timestamps = zeros(1, points);
ref = zeros(1, points);
speed = zeros(1, points);

timestamps(end) = 1;

fig1 = figure(1);

subplot(1, 2, 1)
hold on;
grid on;
plt1 = plot(timestamps, load_in);
plt2 = plot(timestamps, ref);
ylim([-load_max, load_max]);
xlim tight;
ylabel("Load (g)");
xlabel("Timestamps (ms)");

subplot(1, 2, 2)
grid on;
plt3 = plot(timestamps, speed);
ylim([-speed_max, speed_max]);
xlim tight;
ylabel("Motor speed (pps)");
xlabel("Timestamps (ms)");

time_tr_begin = 0;
time_interval_begin = 0;

waitRef10 = true;
waitRef90 = false;
waitInterval = true;
waitOvershoot = true;
waitSettling = true;

sgn = -1;
interval_max = 0;

while(true)
    ser_read = fscanf(ser1);
    reading = str2num(ser_read);

    if(length(reading) == 4)
        %% Plot points
        load_in = [load_in(2:end) reading(1)];
        timestamps = [timestamps(2:end) reading(2)];
        ref = [ref(2:end) reading(3)];
        speed = [speed(2:end) reading(4)];

        set(plt1, 'XData', timestamps, 'YData', load_in);
        set(plt2, 'XData', timestamps, 'YData', ref);
        set(plt3, 'XData', timestamps, 'YData', speed);

        %% Calculate rise time Tr
        if(waitRef10)
            bkgrd = mean(load_in(end-20:end));
        end

        if(waitRef10 && load_in(end) > bkgrd + 0.1*ref(end))
            waitRef10 = false;
            waitRef90 = true;
            time_tr_begin = timestamps(end);
        end

        if(load_in(end) > 0.9*ref(end) && ~waitRef10 && waitRef90)
            waitRef90 = false;
            Tr = timestamps(end) - time_tr_begin;
            disp("Tr: " + Tr);
        end

        %% Calculate settling time Ts

        if(waitSettling)
            sgn_old = sgn;
            sgn = (load_in(end)-ref(end))/abs(load_in(end)-ref(end));

            if(sgn_old ~= sgn && waitInterval)
                waitInterval = false;
                time_interval_begin = timestamps(end);

            elseif(sgn_old ~= sgn && ~waitInterval)
                waitInterval = true;

                interval_max_old = interval_max;
                i_interval_begin = find(timestamps == time_interval_begin);
                interval_max = abs(max(load_in(i_interval_begin:end)));

                if(interval_max < 1.05*ref(end) && interval_max_old < 1.05*ref(end))
                    waitSettling = false;
                    Ts = timestamps(end) - time_tr_begin;
                    disp("Ts: " + Ts)
                end

                %% Calculate overshoot M (first peak only)
                if(waitOvershoot)
                    waitOvershoot = false;
                    M = (interval_max - ref(end))/ref(end);
                    disp("M: " + M*100);
                end
            end
        end

        %% Reset all switches
        if(~waitRef10 && ~waitRef90 && load_in(end) < bkgrd+0.1*ref(end))
            waitRef10 = true;
            waitRef90 = false;
            waitOvershoot = true;
            waitSettling = true;
        end
    end
end

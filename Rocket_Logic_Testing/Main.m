%From open rocket: time (s), h (m), a (m/s^2), T (C), P(mbar)
data = csvread('OpenRocketSim.csv');
//plot(data(:,1),data(:,5));

%Add data padding before launch (10 sec ; timestep .5 sec)
data(:,1) = data(:,1)+10;
initial = data(1,:);
for i = 9.5:-.5:0;
    initial(1:1) = i;
    data = vertcat(initial,data);
end

%Adding mock safety switch engaging at 7 sec
%0 - disengaged ; 1 - engaged
engagedCol = vertcat(zeros(15,1), ones(size(data,1)-15,1));
data = horzcat(data, engagedCol);


%export to csv
csvwrite('test1.csv',data);
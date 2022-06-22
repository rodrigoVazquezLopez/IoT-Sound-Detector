A = readmatrix("gaussdata.txt");
B = readmatrix("rawdata.txt");

plot(B(:,1), B(:,2),"LineWidth",1)
hold on
plot(A(:,1), A(:,2),"LineWidth",2)
xlabel("samples")
ylabel("dbA")
legend("raw input","gauss filter")

figure
t = datetime(feeds,'InputFormat','uuuu-MM-dd''T''HH:mm:ssXXX','TimeZone','local')
st = datestr(t)
ts1 = timeseries(field1, st)
ts1.name = "dBA"
%ts1.TimeInfo.Units = "seconds"
%ts1.TimeInfo.StartDate = "2022-06-09 18:15:14"
ts1.TimeInfo.Format = 'dd-mmm HH:MM'
%ts1.Time = ts1.Time - ts1.Time(30);


plot(ts1)
ylim([30 60])
tstart = datetime("09-Jun-2022 20:12:25");
tend = datetime("12-Jun-2022 17:20:38");
xlim([tstart tend])
title("")

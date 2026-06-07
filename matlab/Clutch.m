clc, clear all

%Rigid calmping shaft coupling 

%M3 8.8 Iso-grovgänga data:
sigma_s = 640*10^6; %Stäckgräns [Pa]
dmin = 2.459*10^-3; %Min-diameter [m]
beta_deg = 60;      %Profilvinkel
dm = 2.675*10^-3;   %Medeldiameter [m] 
p = 0.5*10^-3;      %Stigning [m]
my_g = 0.25;         %Friktionskoefficient i gängan (obehandlad skruv)

%Checka dem här 
D_hal = 3.6*10^-3;    %Håldiameter [m] (ändra sen)
N = 5.5*10^-3;        %Nyckelvidd [m] (ändra sen) 

%Övrigt:
sf = 1.4;    %Framtagen säkerhetsfaktor 
n = 4;       %Antal skruvar i koppling
my = 0.5;    %Friktionskoefficent mellan aluminium och stål
d = 8*10^-3; %Håldiameter koppling [m]

%%
% Åtrdragningsmoment för skruvar
%Beräkningar:
tan_alpha = p/(pi*dm);

%Beta skillt från noll -- justerar friktionskoefficienten
beta_rad = deg2rad(beta_deg);
theta = beta_rad/2;
my_g_prim = my_g/cos(theta);

%Medeldiameter för skruvskallens och kontaktyta med underlaget
D = (D_hal+N)/2;

%s.66 Handbok rekomenderar åtspänning som motsvarar 0.7 av sträckgräns:
sigma_i = 0.7*sigma_s;
Fa = sigma_i*((dmin/2)^2)*pi;

%s.64 Vridmoment omvandlas till axialkraft
M = Fa*dm/2*((tan_alpha+my_g_prim)/(1-my_g_prim*tan_alpha)+my_g_prim*D/dm);

disp(M)

%%
%Maximalt överförbart moment
Fs = sigma_s*((dmin/2)^2)*pi;
F_till = Fs/sf;
F_klam = n*F_till;
Ff = F_klam*my;
M_max = Ff*d/2;

disp(M_max)

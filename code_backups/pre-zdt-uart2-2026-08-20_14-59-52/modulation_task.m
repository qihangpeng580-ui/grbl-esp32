%% ========================================================================
%  Communication Systems Modulation Lab
%  Task: Triangle signal modulation (DSB, SSB, FM, PM)
%  - Triangle signal: 500Hz, 0-2V amplitude
%  - Carrier: 100kHz cosine
%  ========================================================================

clear; clc; close all;

%% ======================== Parameters ====================================
fs = 1e6;                   % Sampling frequency (1 MHz >> 100kHz carrier)
Ts = 1/fs;                  % Sampling period
t_total = 0.01;             % Total time duration (10 ms = 5 cycles of 500Hz)
t = 0:Ts:t_total-Ts;        % Time vector
N = length(t);              % Number of samples

f_tri = 500;                % Triangle wave frequency (500 Hz)
f_c   = 100e3;              % Carrier frequency (100 kHz)
A_tri = 2;                  % Triangle wave amplitude (0 to 2V)

output_dir = 'D:\EE202A2\';  % Output directory for saving figures
if ~exist(output_dir, 'dir')
    mkdir(output_dir);
end

% PSD parameters for Welch method (continuous spectrum)
nfft = 4096;                % FFT points for PSD
window = hamming(512);      % Window function for PSD
noverlap = 256;             % Overlap between segments

%% ================ 1. Generate Triangle Signal ===========================
% sawtooth(t, width) with width=0.5 gives symmetric triangle (-1~+1)
% Scale to 0~2V: (-1+1)/2*2 = 0, (1+1)/2*2 = 2
triangle_signal = A_tri * (sawtooth(2*pi*f_tri*t, 0.5) + 1) / 2;

%% ================ 2. Carrier Signal =====================================
carrier = cos(2*pi*f_c*t);   % Cosine carrier at 100 kHz

%% ================ a) Continuous PSD of Triangle Signal =================
% Use Welch's PSD estimate for continuous-looking spectrum
[pxx_tri, f_psd] = pwelch(triangle_signal, window, noverlap, nfft, fs);

figure('Name', 'Triangle Signal', 'Position', [100, 100, 1200, 500]);

subplot(1,2,1);
plot(t*1000, triangle_signal, 'b-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Amplitude (V)');
title('Triangle Signal (500 Hz, 0-2 V)');
grid on; xlim([0, 5]);

subplot(1,2,2);
plot(f_psd/1000, 10*log10(pxx_tri), 'b-', 'LineWidth', 1.5);
xlabel('Frequency (kHz)'); ylabel('PSD (dB/Hz)');
title('Power Spectral Density of Triangle Signal');
grid on; xlim([0, 20]);
saveas(gcf, [output_dir, 'a_triangle_signal_psd.png']);

% Also compute FFT for verification of harmonic amplitudes
Y_tri = fft(triangle_signal);
Y_tri_mag = abs(Y_tri/N);
Y_tri_mag = Y_tri_mag(1:N/2+1);
Y_tri_mag(2:end-1) = 2*Y_tri_mag(2:end-1);
f_axis = fs*(0:(N/2))/N;

% Verify: theoretical fundamental peak = 8/pi^2 = 0.8106
fprintf('\n--- Triangle Spectrum Verification ---\n');
[~, idx_fund] = min(abs(f_axis - f_tri));
fprintf('Fundamental (500Hz):  %.4f (theoretical: 8/pi^2 = %.4f)\n', ...
    Y_tri_mag(idx_fund), 8/pi^2);
[~, idx_h3] = min(abs(f_axis - 3*f_tri));
fprintf('3rd harmonic (1500Hz): %.4f (theoretical: 8/(9*pi^2) = %.4f)\n', ...
    Y_tri_mag(idx_h3), 8/(9*pi^2));
[~, idx_h5] = min(abs(f_axis - 5*f_tri));
fprintf('5th harmonic (2500Hz): %.4f (theoretical: 8/(25*pi^2) = %.4f)\n', ...
    Y_tri_mag(idx_h5), 8/(25*pi^2));
[~, idx_dc] = min(abs(f_axis - 0));
fprintf('DC component (0Hz):   %.4f (expected: 1.0000)\n', Y_tri_mag(idx_dc));
fprintf('------------------------------------\n');

%% ================ b) DSB Modulation and Spectrum =======================
% DSB-SC modulation: m(t) * cos(2*pi*fc*t)
% Remove DC component from triangle for proper DSB-SC
triangle_ac = triangle_signal - mean(triangle_signal);

dsb_signal = triangle_ac .* carrier;

% PSD of DSB signal (continuous spectrum via Welch)
[pxx_dsb, f_psd2] = pwelch(dsb_signal, window, noverlap, nfft, fs);

figure('Name', 'DSB Modulation', 'Position', [100, 100, 1400, 600]);

subplot(2,2,1);
plot(t*1000, triangle_ac, 'b-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Amplitude (V)');
title('Message Signal (AC coupled)'); grid on; xlim([0, 5]);

subplot(2,2,2);
plot(t*1000, carrier, 'r-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Amplitude (V)');
title('Carrier Signal (100 kHz)'); grid on; xlim([0, 0.1]);

subplot(2,2,3);
plot(t*1000, dsb_signal, 'g-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Amplitude (V)');
title('DSB-SC Modulated Signal'); grid on; xlim([0, 1]);

subplot(2,2,4);
plot(f_psd2/1000, 10*log10(pxx_dsb), 'g-', 'LineWidth', 1.5);
xlabel('Frequency (kHz)'); ylabel('PSD (dB/Hz)');
title('Power Spectral Density of DSB-SC Signal'); grid on;
xlim([80, 120]);
saveas(gcf, [output_dir, 'b_dsb_modulation_spectrum.png']);

%% ================ c) SSB Modulation using Hilbert Transform ============
% SSB(t) = m(t)*cos(2*pi*fc*t) - mh(t)*sin(2*pi*fc*t)
% where mh(t) is the Hilbert transform of m(t) (upper sideband)

mh = imag(hilbert(triangle_ac));  % Hilbert transform of message

% Generate SSB (upper sideband)
ssb_usb = triangle_ac .* cos(2*pi*f_c*t) - mh .* sin(2*pi*f_c*t);

% PSD of SSB signal
[pxx_ssb, f_psd3] = pwelch(ssb_usb, window, noverlap, nfft, fs);

figure('Name', 'SSB Modulation', 'Position', [100, 100, 1200, 500]);

subplot(1,2,1);
plot(t*1000, ssb_usb, 'm-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Amplitude (V)');
title('SSB (Upper Sideband) Modulated Signal');
grid on; xlim([0, 1]);

subplot(1,2,2);
plot(f_psd3/1000, 10*log10(pxx_ssb), 'm-', 'LineWidth', 1.5);
xlabel('Frequency (kHz)'); ylabel('PSD (dB/Hz)');
title('Power Spectral Density of SSB (USB) Signal');
grid on; xlim([95, 115]);
saveas(gcf, [output_dir, 'c_ssb_modulation_spectrum.png']);

%% ================ d) FM Modulation (Modulation Index = 10) =============
% FM: s(t) = cos(2*pi*fc*t + 2*pi*kf * integral(m(tau) dtau))

beta_fm = 10;  % Modulation index

message_int = cumsum(triangle_ac) * Ts;  % Numerical integration of message
kf = beta_fm * 2*pi*f_tri / max(abs(message_int));  % Freq deviation constant

fm_signal = cos(2*pi*f_c*t + 2*pi*kf * message_int);
fm_signal = fm_signal / max(abs(fm_signal));

% PSD of FM signal
[pxx_fm, f_psd4] = pwelch(fm_signal, window, noverlap, nfft, fs);

figure('Name', 'FM Modulation', 'Position', [100, 100, 1400, 600]);

subplot(2,2,1);
plot(t*1000, triangle_ac, 'b-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Amplitude (V)');
title('Message Signal'); grid on; xlim([0, 5]);

subplot(2,2,2);
plot(t*1000, message_int*1000, 'r-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Integral (V\cdotms)');
title('Integral of Message (Phase deviation)'); grid on; xlim([0, 5]);

subplot(2,2,3);
plot(t*1000, fm_signal, 'c-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Amplitude');
title('FM Signal (\beta = 10)'); grid on; xlim([0, 2]);

subplot(2,2,4);
plot(f_psd4/1000, 10*log10(pxx_fm), 'c-', 'LineWidth', 1.5);
xlabel('Frequency (kHz)'); ylabel('PSD (dB/Hz)');
title('Power Spectral Density of FM Signal (\beta = 10)'); grid on;
xlim([70, 130]);
saveas(gcf, [output_dir, 'd_fm_modulation_spectrum.png']);

%% ================ e) PM Modulation (Modulation Index = 10) =============
% PM: s(t) = cos(2*pi*fc*t + kp * m(t))

beta_pm = 10;  % Modulation index
kp = beta_pm / max(abs(triangle_ac));  % Phase sensitivity

pm_signal = cos(2*pi*f_c*t + kp * triangle_ac);
pm_signal = pm_signal / max(abs(pm_signal));

% PSD of PM signal
[pxx_pm, f_psd5] = pwelch(pm_signal, window, noverlap, nfft, fs);

figure('Name', 'PM Modulation', 'Position', [100, 100, 1400, 600]);

subplot(2,2,1);
plot(t*1000, triangle_ac, 'b-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Amplitude (V)');
title('Message Signal'); grid on; xlim([0, 5]);

subplot(2,2,2);
plot(t*1000, kp*triangle_ac, 'r-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Phase Deviation');
title('Phase Deviation (kp \times m(t))'); grid on; xlim([0, 5]);

subplot(2,2,3);
plot(t*1000, pm_signal, 'k-', 'LineWidth', 1.5);
xlabel('Time (ms)'); ylabel('Amplitude');
title('PM Signal (\beta = 10)'); grid on; xlim([0, 2]);

subplot(2,2,4);
plot(f_psd5/1000, 10*log10(pxx_pm), 'k-', 'LineWidth', 1.5);
xlabel('Frequency (kHz)'); ylabel('PSD (dB/Hz)');
title('Power Spectral Density of PM Signal (\beta = 10)'); grid on;
xlim([70, 130]);
saveas(gcf, [output_dir, 'e_pm_modulation_spectrum.png']);

%% ======================== Summary ======================================
fprintf('========================= Summary =========================\n');
fprintf('Triangle Signal Frequency : %d Hz\n', f_tri);
fprintf('Carrier Frequency         : %d kHz\n', f_c/1000);
fprintf('Sampling Frequency        : %d MHz\n', fs/1e6);
fprintf('FFT Resolution            : %.1f Hz\n', fs/N);
fprintf('============================================================\n');
fprintf('Modulation Types Generated:\n');
fprintf('  1. Triangle signal spectrum (a)\n');
fprintf('  2. DSB-SC modulated signal and spectrum (b)\n');
fprintf('  3. SSB (USB) modulated signal and spectrum (c)\n');
fprintf('  4. FM modulated signal and spectrum (d) - beta = %d\n', beta_fm);
fprintf('  5. PM modulated signal and spectrum (e) - beta = %d\n', beta_pm);
fprintf('============================================================\n');

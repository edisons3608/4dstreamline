% show heatmap of vel mag over cardiac cycle

% Load the velocity data
vel = load("/Users/edisonsun/Documents/4Dsamples/D71/4D/3dpc_20250625_152126/vel_struct.mat").mrStruct;

% Extract velocity data
if isfield(vel, 'dataAy')
    velocity_data = vel.dataAy;
else
    error('Velocity data not found in the structure');
end

% Get data dimensions
[dim1, dim2, dim3, dim4, dim5] = size(velocity_data);
fprintf('Data dimensions: %d x %d x %d x %d x %d\n', dim1, dim2, dim3, dim4, dim5);

% Extract velocity components

% b/c vel comes in 5D and doesn't remove 4th dim even when indexing b/c
% lodged bwt dims
vx = squeeze(velocity_data(:,:,:,1,:)); % x-component
vy = squeeze(velocity_data(:,:,:,2,:)); % y-component  
vz = squeeze(velocity_data(:,:,:,3,:)); % z-component

% Calculate velocity magnitude
velocity_magnitude = sqrt(vx.^2 + vy.^2 + vz.^2);

% Create time vector (assuming cardiac cycle)
time_points = 1:dim5;
cardiac_cycle = linspace(0, 100, dim5); % Percentage of cardiac cycle

% Create figure for time series visualization
figure('Position', [100, 100, 1200, 800]);

% Plot 1: Velocity magnitude over time at center point
subplot(2,3,1);
center_x = round(dim1/2);
center_y = round(dim2/2);
center_z = round(dim3/2);
center_vel = squeeze(velocity_magnitude(center_x, center_y, center_z, :));
plot(cardiac_cycle, center_vel, 'b-', 'LineWidth', 2);
xlabel('Cardiac Cycle (%)');
ylabel('Velocity Magnitude (m/s)');
title('Velocity at Center Point');
grid on;

% Plot 2: Velocity components over time at center point
subplot(2,3,2);
center_vx = squeeze(vx(center_x, center_y, center_z, :));
center_vy = squeeze(vy(center_x, center_y, center_z, :));
center_vz = squeeze(vz(center_x, center_y, center_z, :));
plot(cardiac_cycle, center_vx, 'r-', 'LineWidth', 2, 'DisplayName', 'Vx');
hold on;
plot(cardiac_cycle, center_vy, 'g-', 'LineWidth', 2, 'DisplayName', 'Vy');
plot(cardiac_cycle, center_vz, 'b-', 'LineWidth', 2, 'DisplayName', 'Vz');
xlabel('Cardiac Cycle (%)');
ylabel('Velocity (m/s)');
title('Velocity Components at Center Point');
legend('show');
grid on;

% Plot 3: Mean velocity magnitude over time
subplot(2,3,3);
mean_vel = squeeze(mean(mean(mean(velocity_magnitude, 1), 2), 3));
plot(cardiac_cycle, mean_vel, 'k-', 'LineWidth', 2);
xlabel('Cardiac Cycle (%)');
ylabel('Mean Velocity Magnitude (m/s)');
title('Mean Velocity Over Time');
grid on;

% Plot 4: Maximum velocity over time
subplot(2,3,4);
max_vel = squeeze(max(max(max(velocity_magnitude, [], 1), [], 2), [], 3));
plot(cardiac_cycle, max_vel, 'm-', 'LineWidth', 2);
xlabel('Cardiac Cycle (%)');
ylabel('Max Velocity Magnitude (m/s)');
title('Maximum Velocity Over Time');
grid on;

% Plot 5: Velocity distribution at peak time
subplot(2,3,5);
[~, peak_time] = max(mean_vel);
peak_vel = squeeze(velocity_magnitude(:,:,:,peak_time));
peak_vel_flat = peak_vel(:);
histogram(peak_vel_flat, 50, 'FaceAlpha', 0.7);
xlabel('Velocity Magnitude (m/s)');
ylabel('Frequency');
title(sprintf('Velocity Distribution at Peak Time (%d)', peak_time));
grid on;

% Plot 6: Velocity magnitude heatmap at center slice
subplot(2,3,6);
center_slice = squeeze(velocity_magnitude(:,:,center_z,peak_time));
imagesc(center_slice);
colorbar;
xlabel('X dimension');
ylabel('Y dimension');
title(sprintf('Velocity Magnitude at Peak Time (Slice %d)', center_z));
axis equal tight;

% Create animation of velocity magnitude over time
figure('Position', [200, 200, 800, 600]);
for t = 1:dim5
    % Get current time slice
    current_vel = squeeze(velocity_magnitude(:,:,center_z,t));
    
    % Display
    imagesc(current_vel);
    colorbar;
    title(sprintf('Velocity Magnitude - Time Point %d/%d (%.1f%% of cardiac cycle)', ...
          t, dim5, cardiac_cycle(t)));
    xlabel('X dimension');
    ylabel('Y dimension');
    axis equal tight;
    
    % Add pause for animation
    pause(0.1);
    
    % Clear figure for next frame
    if t < dim5
        clf;
    end
end

% Display summary statistics
fprintf('\n=== 4D Flow Data Summary ===\n');
fprintf('Spatial dimensions: %d x %d x %d\n', dim1, dim2, dim3);
fprintf('Velocity components: %d (x, y, z)\n', dim4);
fprintf('Time points: %d\n', dim5);
fprintf('Peak velocity: %.3f m/s\n', max(max_vel));
fprintf('Mean peak velocity: %.3f m/s\n', max(mean_vel));
fprintf('Peak time point: %d\n', peak_time);
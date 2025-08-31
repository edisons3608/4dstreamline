
% show vel field instead of streamlines
% Load the velocity data
vel = load("/Users/edisonsun/Documents/4Dsamples/D71/4D/3dpc_20250625_152126/vel_struct.mat").mrStruct;
mask = load("/Users/edisonsun/Documents/4Dsamples/D71/4D/3dpc_20250625_152126/aorta_mask_struct.mat").mrStruct;
% Extract velocity data
if isfield(vel, 'dataAy')
    velocity_data = vel.dataAy;
else
    error('Velocity data not found in the structure');
end

% Extract mask data
if isfield(mask, 'dataAy')
    mask_data = mask.dataAy;
else
    error('Mask data not found in the structure');
end

% Get data dimensions
[dim1, dim2, dim3, dim4, dim5] = size(velocity_data);
fprintf('Data dimensions: %d x %d x %d x %d x %d\n', dim1, dim2, dim3, dim4, dim5);

% Get mask dimensions and ensure it matches velocity data
[mask_dim1, mask_dim2, mask_dim3, mask_dim4, mask_dim5] = size(mask_data);
fprintf('Mask dimensions: %d x %d x %d x %d x %d\n', mask_dim1, mask_dim2, mask_dim3, mask_dim4, mask_dim5);


% Create coordinate grids for 3D visualization
[X, Y, Z] = meshgrid(1:dim2, 1:dim1, 1:dim3);

% Set up figure for visualization
figure('Position', [100, 100, 1200, 800]);

% Loop through each frame in the cardiac cycle
for frame = 1:dim5
    fprintf('Processing frame %d of %d\n', frame, dim5);
    
    % Extract velocity data for current frame
    % Assuming velocity_data has 3 components (x, y, z) in dim4
    if dim4 == 3
        vx = squeeze(velocity_data(:, :, :, 1, frame));
        vy = squeeze(velocity_data(:, :, :, 2, frame));
        vz = squeeze(velocity_data(:, :, :, 3, frame));
    else
        % If only one velocity component
        vx = squeeze(velocity_data(:, :, :, 1, frame));
        vy = zeros(size(vx));
        vz = zeros(size(vx));
    end
    
    % Extract mask (constant across all frames)
    current_mask = squeeze(mask_data(:, :, :, 1, 1));
    
    % Apply mask to velocity data
    vx_masked = vx .* current_mask;
    vy_masked = vy .* current_mask;
    vz_masked = vz .* current_mask;
    
    % Calculate velocity magnitude
    velocity_magnitude = sqrt(vx_masked.^2 + vy_masked.^2 + vz_masked.^2);
    
    % Clear previous plot
    clf;
    
    % Create subplot layout
    subplot(2, 3, 1);
    % Show velocity magnitude in middle slice
    mid_slice = round(dim3/2);
    imagesc(squeeze(velocity_magnitude(:, :, mid_slice)));
    colorbar;
    title(sprintf('Frame %d: Velocity Magnitude (Slice %d)', frame, mid_slice));
    axis equal tight;
    
    subplot(2, 3, 2);
    % Show x-component velocity
    imagesc(squeeze(vx_masked(:, :, mid_slice)));
    colorbar;
    title(sprintf('Frame %d: X-Velocity (Slice %d)', frame, mid_slice));
    axis equal tight;
    
    subplot(2, 3, 3);
    % Show y-component velocity
    imagesc(squeeze(vy_masked(:, :, mid_slice)));
    colorbar;
    title(sprintf('Frame %d: Y-Velocity (Slice %d)', frame, mid_slice));
    axis equal tight;
    
    subplot(2, 3, 4);
    % Show z-component velocity
    imagesc(squeeze(vz_masked(:, :, mid_slice)));
    colorbar;
    title(sprintf('Frame %d: Z-Velocity (Slice %d)', frame, mid_slice));
    axis equal tight;
    
    subplot(2, 3, 5);
    % Show mask
    imagesc(squeeze(current_mask(:, :, mid_slice)));
    colorbar;
    title(sprintf('Frame %d: Mask (Slice %d)', frame, mid_slice));
    axis equal tight;
    
    subplot(2, 3, 6);
    % 3D quiver plot of velocity field (downsampled for clarity)
    downsample_factor = 4;
    X_ds = X(1:downsample_factor:end, 1:downsample_factor:end, 1:downsample_factor:end);
    Y_ds = Y(1:downsample_factor:end, 1:downsample_factor:end, 1:downsample_factor:end);
    Z_ds = Z(1:downsample_factor:end, 1:downsample_factor:end, 1:downsample_factor:end);
    vx_ds = vx_masked(1:downsample_factor:end, 1:downsample_factor:end, 1:downsample_factor:end);
    vy_ds = vy_masked(1:downsample_factor:end, 1:downsample_factor:end, 1:downsample_factor:end);
    vz_ds = vz_masked(1:downsample_factor:end, 1:downsample_factor:end, 1:downsample_factor:end);
    
    % Only show vectors where mask is non-zero
    mask_ds = current_mask(1:downsample_factor:end, 1:downsample_factor:end, 1:downsample_factor:end);
    valid_indices = mask_ds > 0;
    
    if any(valid_indices(:))
        quiver3(X_ds(valid_indices), Y_ds(valid_indices), Z_ds(valid_indices), ...
                vx_ds(valid_indices), vy_ds(valid_indices), vz_ds(valid_indices), 2, 'b');
    end
    title(sprintf('Frame %d: 3D Velocity Field', frame));
    xlabel('X'); ylabel('Y'); zlabel('Z');
    axis equal;
    grid on;
    
    % Add overall title
    sgtitle(sprintf('Velocity Field Visualization - Cardiac Cycle Frame %d/%d', frame, dim5));
    
    % Pause to show the frame (adjust timing as needed)
    pause(0.5);
    
    % Optional: save frame as image
    % saveas(gcf, sprintf('velocity_frame_%03d.png', frame));
end

fprintf('Velocity field visualization complete for all %d frames\n', dim5);

% Additional analysis: Calculate statistics for each frame
fprintf('\nVelocity Statistics by Frame:\n');
fprintf('Frame\tMean Vel\tMax Vel\tMin Vel\tStd Vel\n');
fprintf('-----\t--------\t-------\t-------\t-------\n');

for frame = 1:dim5
    if dim4 == 3
        vx = squeeze(velocity_data(:, :, :, 1, frame));
        vy = squeeze(velocity_data(:, :, :, 2, frame));
        vz = squeeze(velocity_data(:, :, :, 3, frame));
    else
        vx = squeeze(velocity_data(:, :, :, 1, frame));
        vy = zeros(size(vx));
        vz = zeros(size(vx));
    end
    
    current_mask = squeeze(mask_data(:, :, :, 1, 1));
    velocity_magnitude = sqrt((vx .* current_mask).^2 + (vy .* current_mask).^2 + (vz .* current_mask).^2);
    
    % Calculate statistics only within masked region
    masked_vel = velocity_magnitude(current_mask > 0);
    if ~isempty(masked_vel)
        mean_vel = mean(masked_vel(:));
        max_vel = max(masked_vel(:));
        min_vel = min(masked_vel(:));
        std_vel = std(masked_vel(:));
        fprintf('%d\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\n', frame, mean_vel, max_vel, min_vel, std_vel);
    else
        fprintf('%d\tN/A\t\tN/A\t\tN/A\t\tN/A\n', frame);
    end
end

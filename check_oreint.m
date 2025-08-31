% check orientation to ensure vel struct is same orientation on mask

vel = load("/Users/edisonsun/Documents/4Dsamples/D71/4D/3dpc_20250625_152126/vel_struct.mat").mrStruct;
mask = load("/Users/edisonsun/Documents/4Dsamples/D71/4D/3dpc_20250625_152126/aorta_mask_struct.mat").mrStruct;

% Get velocity components
vel_x = vel.dataAy(:,:,:,1);
vel_y = vel.dataAy(:,:,:,2);
vel_z = vel.dataAy(:,:,:,3);

% Calculate velocity magnitude
vel_mag = sqrt(vel_x.^2 + vel_y.^2 + vel_z.^2);

% Get mask data
mask_data = mask.dataAy;

% Create figure for visualization
figure('Position', [100, 100, 1400, 1000]);

% Get dimensions
[ny, nx, nz] = size(vel_mag);

% Create subplots for different slices
slice_positions = [round(nz/8), round(nz/4), round(3*nz/8), round(nz/2), round(5*nz/8), round(3*nz/4), round(7*nz/8)];

for i = 1:7
    subplot(3, 3, i);
    
    % Extract slice
    slice_idx = slice_positions(i);
    vel_x_slice = vel_x(:,:,slice_idx);
    vel_y_slice = vel_y(:,:,slice_idx);
    vel_mag_slice = vel_mag(:,:,slice_idx);
    mask_slice = mask_data(:,:,slice_idx);
    
    % Create RGB image -------- red channel for velocity magnitude, green channel for mask
    rgb_img = zeros(size(vel_mag_slice, 1), size(vel_mag_slice, 2), 3);
    rgb_img(:,:,1) = vel_mag_slice / max(vel_mag(:)); % Red channel for velocity magnitude
    rgb_img(:,:,2) = mask_slice; % Green channel for mask
    
    imagesc(rgb_img);
    hold on;
    
    % Overlay velocity vectors (quiver plot)
    % Sample every few pixels to avoid cluttering
    step = 4;
    [X, Y] = meshgrid(1:step:nx, 1:step:ny);
    U = vel_x_slice(1:step:end, 1:step:end);
    V = vel_y_slice(1:step:end, 1:step:end);
    
    % Only show vectors where mask is present
    mask_sampled = mask_slice(1:step:end, 1:step:end);
    valid_vectors = mask_sampled > 0.5;
    
    quiver(X(valid_vectors), Y(valid_vectors), U(valid_vectors), V(valid_vectors), ...
           'w', 'LineWidth', 1, 'MaxHeadSize', 0.5);
    
    axis equal tight;
    title(sprintf('Slice %d (z=%d) - Velocity Field', i, slice_idx));
    colorbar;
end

% Also show 3D visualization
subplot(3, 3, [8, 9]);
% Create isosurface of mask
[x, y, z] = meshgrid(1:nx, 1:ny, 1:nz);
mask_iso = isosurface(x, y, z, mask_data, 0.5);

% Plot mask surface
p1 = patch(mask_iso, 'FaceColor', 'green', 'EdgeColor', 'none', 'FaceAlpha', 0.3);
hold on;

% Sample points for 3D velocity vectors (to avoid too many vectors)
step_3d = 8;
mask_indices = find(mask_data(1:step_3d:end, 1:step_3d:end, 1:step_3d:end) > 0.5);
[my, mx, mz] = ind2sub(size(mask_data(1:step_3d:end, 1:step_3d:end, 1:step_3d:end)), mask_indices);

% Convert back to full resolution indices
mx = (mx - 1) * step_3d + 1;
my = (my - 1) * step_3d + 1;
mz = (mz - 1) * step_3d + 1;

% Get velocity vectors at these points
vel_x_3d = vel_x(sub2ind(size(vel_x), my, mx, mz));
vel_y_3d = vel_y(sub2ind(size(vel_y), my, mx, mz));
vel_z_3d = vel_z(sub2ind(size(vel_z), my, mx, mz));
vel_mag_3d = vel_mag(sub2ind(size(vel_mag), my, mx, mz));

% Plot 3D velocity vectors
quiver3(mx, my, mz, vel_x_3d, vel_y_3d, vel_z_3d, 2, 'LineWidth', 1, 'MaxHeadSize', 0.5, 'Color', 'red');

colormap('jet');
colorbar;
title('3D View: Velocity Field on Mask');
xlabel('X'); ylabel('Y'); zlabel('Z');
view(3);
grid on;

% Add some stats
fprintf('Velocity magnitude range: %.2f to %.2f\n', min(vel_mag(:)), max(vel_mag(:)));
fprintf('Mask volume: %d voxels\n', sum(mask_data(:) > 0.5));
fprintf('Mean velocity in mask: %.2f\n', mean(vel_mag(mask_data > 0.5)));


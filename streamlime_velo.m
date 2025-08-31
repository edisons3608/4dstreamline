% show streamlines for each frame

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

% Ensure mask and velocity data have the same spatial dimensions
if dim1 ~= mask_dim1 || dim2 ~= mask_dim2 || dim3 ~= mask_dim3
    error('Mask and velocity data have different spatial dimensions');
end

% Display streamline velocity over the cardiac cycle
figure('Position', [100, 100, 1200, 800]);

% Create a grid for streamline visualization
[x, y, z] = meshgrid(1:dim2, 1:dim1, 1:dim3);

% Define starting points for streamlines within the mask
% Get mask for the first frame to find valid starting points
if mask_dim5 == 1
    current_mask = squeeze(mask_data(:, :, :, 1, 1));
else
    current_mask = squeeze(mask_data(:, :, :, 1, 1));
end

% Find indices where mask is non-zero (vessel region)
[mask_y, mask_x, mask_z] = ind2sub(size(current_mask), find(current_mask > 0));

% Select starting points within the mask
num_start_points = min(20, length(mask_x)); % Limit to 20 points or available points
if num_start_points > 0
    % Randomly select points from within the mask
    rand_indices = randperm(length(mask_x), num_start_points);
    startx = mask_x(rand_indices);
    starty = mask_y(rand_indices);
    startz = mask_z(rand_indices);
else
    % Fallback to center points if no mask points found
    startx = round(dim2/2) * ones(1, 10);
    starty = round(dim1/2) * ones(1, 10);
    startz = linspace(1, dim3, 10);
end

% Loop through all cardiac phases
for frame = 1:dim5

    if dim4 == 3  % If dim4 represents x,y,z velocity components
        vx = squeeze(velocity_data(:, :, :, 1, frame));
        vy = squeeze(velocity_data(:, :, :, 2, frame));
        vz = squeeze(velocity_data(:, :, :, 3, frame));
    else  % If dim4 represents velocity magnitude
        v_mag = squeeze(velocity_data(:, :, :, 1, frame));

        vx = v_mag;
        vy = v_mag;
        vz = v_mag;
    end
    

    if mask_dim5 == 1
        current_mask = squeeze(mask_data(:, :, :, 1, 1));
    else
        current_mask = squeeze(mask_data(:, :, :, 1, frame));
    end
    
    % Apply mask to velocity field (set velocity to zero outside mask)
    vx = vx .* current_mask;
    vy = vy .* current_mask;
    vz = vz .* current_mask;
    
    % Clear previous frame
    clf;
    
    % Calculate velocity magnitude
    v_magnitude = sqrt(vx.^2 + vy.^2 + vz.^2);
    
    % Create subplot for current frame
    subplot(2, 2, 1);
    imagesc(squeeze(vx(:, :, round(dim3/2))));
    title(sprintf('Vx (Masked) - Frame %d/%d', frame, dim5));
    colorbar;
    axis equal tight;
    
    subplot(2, 2, 2);
    imagesc(squeeze(vy(:, :, round(dim3/2))));
    title(sprintf('Vy (Masked) - Frame %d/%d', frame, dim5));
    colorbar;
    axis equal tight;
    
    subplot(2, 2, 3);
    imagesc(squeeze(vz(:, :, round(dim3/2))));
    title(sprintf('Vz (Masked) - Frame %d/%d', frame, dim5));
    colorbar;
    axis equal tight;
    
    subplot(2, 2, 4);
    imagesc(squeeze(v_magnitude(:, :, round(dim3/2))));
    title(sprintf('Velocity Magnitude (Masked) - Frame %d/%d', frame, dim5));
    colorbar;
    axis equal tight;
    
    % Add streamline visualization in 3D
    figure(2);
    clf;
    
    % Create 3D streamline plot within mask
    streamline(x, y, z, vx, vy, vz, startx, starty, startz);
    hold on;
    
    % Display the mask as a semi-transparent surface
    [mask_surf_x, mask_surf_y, mask_surf_z] = meshgrid(1:dim2, 1:dim1, 1:dim3);
    mask_surf = isosurface(mask_surf_x, mask_surf_y, mask_surf_z, current_mask, 0.5);
    if ~isempty(mask_surf.vertices)
        patch(mask_surf, 'FaceColor', 'red', 'EdgeColor', 'none', 'FaceAlpha', 0.1);
    end
    
    % Add velocity magnitude as color on slices
    slice(x, y, z, v_magnitude, [], [], []);
    colormap(jet);
    colorbar;
    
    title(sprintf('3D Streamlines (Masked) - Frame %d/%d', frame, dim5));
    xlabel('X'); ylabel('Y'); zlabel('Z');
    axis equal;
    view(3);
    lighting gouraud;
    camlight;

    pause(0.5);
    

end

fprintf('Streamline visualization complete for %d cardiac phases\n', dim5);
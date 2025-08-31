% Simple 4D Flow Streamline Visualization Script
% Reads velocity components from DICOM folders and creates streamlines

% Example (replace with your folders):
folder = "D71";
vxDir = "/Users/edisonsun/Documents/4Dsamples/"+folder+"/4D/1";
vyDir = "/Users/edisonsun/Documents/4Dsamples/"+folder+"/4D/2";
vzDir = "/Users/edisonsun/Documents/4Dsamples/"+folder+"/4D/3";
magDir = "/Users/edisonsun/Documents/4Dsamples/"+folder+"/4D/mag";  % optional
mask  = "/Users/edisonsun/Documents/4Dsamples/"+folder+"/4D/3dpc_20250625_152126/Segmentation.nii"
cycle = 20;
x = read4DFlow(vxDir,cycle);
y = read4DFlow(vyDir,cycle);
z = read4DFlow(vzDir,cycle);
mag = read4DFlow(magDir,cycle);

% Read the mask file
mask_data = niftiread(mask);
function out = read4DFlow(dicomFolderPath, cardiacCycles)
    % Read all DICOM files from a folder
    % Input: dicomFolderPath - path to folder containing DICOM files
    %        cardiacCycles - number of cardiac cycles (optional)
    % Output: out - 4D array of DICOM data [height x width x slices x time]
    
    % Get all files in the directory
    files = dir(fullfile(dicomFolderPath, '*.dcm'));
    if isempty(files)
        files = dir(fullfile(dicomFolderPath, '*'));
        files = files(~[files.isdir]);
    end
    
    if isempty(files)
        error('No files found in directory: %s', dicomFolderPath);
    end
    
    % Read first file to get dimensions
    info = dicominfo(fullfile(dicomFolderPath, files(1).name));
    first_img = dicomread(info);
    
    % Get dimensions
    [nx, ny] = size(first_img);
    nt = length(files);

    nz = nt/cardiacCycles;
    
    % Initialize output array
    out = zeros(nx, ny, nz,cardiacCycles);
    
    t = 0;
    % Read all files
    for i = 1:nt
        info = dicominfo(fullfile(dicomFolderPath, files(i).name));
        slice_idx = mod(i-1, nz) + 1;  % 1-based indexing for slice
        cycle_idx = floor((i-1)/nz) + 1;  % 1-based indexing for cycle
        out(:,:,slice_idx,cycle_idx) = dicomread(info);
    end
    
end

%%

rescale_x = x*2-4096;
rescale_y = y*2-4096;
rescale_z = z*2-4096;

%%
% Visualize streamlines for 4D flow data
% Select time frame (1-25)
time_frame = 10; % middle frame

% Extract velocity components and magnitude for selected time frame
Vx = squeeze(rescale_x(:,:,:,time_frame));
Vy = squeeze(rescale_y(:,:,:,time_frame));
Vz = squeeze(rescale_z(:,:,:,time_frame));
%Mag = squeeze(mag(:,:,:,time_frame));

% Create coordinate grids
[X, Y, Z] = meshgrid(1:size(Vx,2), 1:size(Vx,1), 1:size(Vx,3));

% Create figure
figure('Name', '4D Flow Velocity Field', 'Position', [100, 100, 1200, 800]);

% First, create an isosurface of the magnitude data as background
% Calculate threshold for isosurface (e.g., 50% of max value)
%mag_threshold = 0.1 * max(Mag(:));
%p = patch(isosurface(X, Y, Z, Mag, mag_threshold));
%p.FaceColor = 'blue';
%p.EdgeColor = 'none';
%p.FaceAlpha = 0.2; % Very transparent
lighting gouraud;
camlight;

hold on;

% Sample the velocity field for quiver plot (use every nth point to avoid clutter)
sample_rate = 4; % Adjust this to show more or fewer vectors
X_sampled = X(1:sample_rate:end, 1:sample_rate:end, 1:sample_rate:end);
Y_sampled = Y(1:sample_rate:end, 1:sample_rate:end, 1:sample_rate:end);
Z_sampled = Z(1:sample_rate:end, 1:sample_rate:end, 1:sample_rate:end);
Vx_sampled = Vx(1:sample_rate:end, 1:sample_rate:end, 1:sample_rate:end);
Vy_sampled = Vy(1:sample_rate:end, 1:sample_rate:end, 1:sample_rate:end);
Vz_sampled = Vz(1:sample_rate:end, 1:sample_rate:end, 1:sample_rate:end);

% Sample the mask data to match the velocity field sampling
[mask_rows, mask_cols, mask_slices] = size(mask_data);
mask_sampled = mask_data(1:sample_rate:mask_rows, 1:sample_rate:mask_cols, 1:sample_rate:mask_slices);

% Find indices where mask is non-zero (inside the mask)
mask_indices = find(mask_sampled > 0);

% Extract only the velocity vectors inside the mask
X_masked = X_sampled(mask_indices);
Y_masked = Y_sampled(mask_indices);
Z_masked = Z_sampled(mask_indices);
Vx_masked = Vx_sampled(mask_indices);
Vy_masked = Vy_sampled(mask_indices);
Vz_masked = Vz_sampled(mask_indices);

% Plot velocity vectors using quiver3 (only inside mask)
h = quiver3(X_masked, Y_masked, Z_masked, Vx_masked, Vy_masked, Vz_masked, 2, 'r', 'LineWidth', 1.5);
set(h, 'MaxHeadSize', 0.5);

% Add streamlines starting from points inside the mask
% Create starting points for streamlines (use a subset of masked points)
num_streamlines = 50; % Adjust number of streamlines
if length(mask_indices) > num_streamlines
    % Randomly select starting points
    start_indices = mask_indices(randperm(length(mask_indices), num_streamlines));
else
    start_indices = mask_indices;
end

% Get starting coordinates for streamlines
start_x = X_sampled(start_indices);
start_y = Y_sampled(start_indices);
start_z = Z_sampled(start_indices);

% Plot streamlines
streamline(X_sampled, Y_sampled, Z_sampled, Vx_sampled, Vy_sampled, Vz_sampled, start_x, start_y, start_z);

% Set view properties
view(3);
axis equal;
grid on;
xlabel('X');
ylabel('Y');
zlabel('Z');
title(sprintf('4D Flow Velocity Field - Time Frame %d/%d', time_frame, cycle));

hold off;

fprintf('Streamline visualization complete for time frame %d!\n', time_frame);


function pose = extract_roi(img_path)
	im = imread(img_path);
	estimator = uci.PoseEstimator.default_estimator;
	boxes = estimator.estimate(im);
	pose = sbu.Pose(boxes);
	% struct2csv(pose, [img_path(1:end-4) '.csv']);

	% extract upper part
	upper_center_x = round((pose.left_shoulder_x + pose.right_shoulder_x)/2);
	upper_center_y = round((max(pose.left_shoulder_y, pose.right_shoulder_y)+min(pose.left_hip_y, pose.right_hip_y))/2);
	upper_half_width = round((pose.left_shoulder_x - pose.right_shoulder_x)*0.45);
	upper_part = im((upper_center_y-upper_half_width):(upper_center_y+upper_half_width), (upper_center_x-upper_half_width):(upper_center_x+upper_half_width), :);
	imwrite(upper_part, [img_path(1:end-4) '_upper.jpg']);

	% extract sleeve part
	sleeve_half_width = round((pose.left_shoulder_x - pose.right_shoulder_x)*0.3);
	sleeve_region.h_start = pose.left_shoulder_y;
	sleeve_region.h_end = pose.left_hand_y;
	sleeve_region.w_start = min(pose.left_shoulder_x,pose.left_hand_x);
	sleeve_region.w_end = max(pose.left_shoulder_x,pose.left_hand_x)+sleeve_half_width;
	sleeve_part = im(sleeve_region.h_start: sleeve_region.h_end, sleeve_region.w_start: sleeve_region.w_end, :);
	imwrite(sleeve_part, [img_path(1:end-4) '_sleeve.jpg']);

	% extract lower part
	lower_center_x = round((pose.right_hip_x + pose.right_knee_x)/2);
	lower_center_y = pose.right_hip_y + round((pose.right_knee_y - pose.right_hip_y)*0.3);
	lower_half_width = round((pose.left_shoulder_x - pose.right_shoulder_x)*0.2);
	lower_part = im((lower_center_y-lower_half_width):(lower_center_y+lower_half_width), (lower_center_x-lower_half_width):(lower_center_x+lower_half_width), :);
	imwrite(lower_part, [img_path(1:end-4) '_lower.jpg']);

	% highlight roi

	shapeInserter = vision.ShapeInserter('BorderColor','Custom', 'CustomBorderColor', uint8([255 0 0]));
	rectangle = int32([upper_center_x-upper_half_width upper_center_y-upper_half_width 2*upper_half_width 2*upper_half_width;
		lower_center_x-lower_half_width lower_center_y-lower_half_width 2*lower_half_width 2*lower_half_width
		sleeve_region.w_start sleeve_region.h_start sleeve_region.w_end-sleeve_region.w_start sleeve_region.h_end-sleeve_region.h_start]);
	J = step(shapeInserter, im, rectangle);

	shapeInserter = vision.ShapeInserter('Shape','Lines','BorderColor','Custom', 'CustomBorderColor', uint8([100 255 255]));
	l = int32([pose.head_x pose.head_y pose.neck_x pose.neck_y;
			pose.neck_x pose.neck_y pose.right_shoulder_x pose.right_shoulder_y;
			pose.neck_x pose.neck_y pose.left_shoulder_x pose.left_shoulder_y;
			pose.right_shoulder_x pose.right_shoulder_y pose.right_elbow_x pose.right_elbow_y;
			pose.left_shoulder_x pose.left_shoulder_y pose.left_elbow_x pose.left_elbow_y;
			pose.right_shoulder_x pose.right_shoulder_y pose.right_hip_x pose.right_hip_y;
			pose.left_shoulder_x pose.left_shoulder_y pose.left_hip_x pose.left_hip_y;
			pose.right_hand_x pose.right_hand_y pose.right_elbow_x pose.right_elbow_y;
			pose.left_hand_x pose.left_hand_y pose.left_elbow_x pose.left_elbow_y;
			pose.right_knee_x pose.right_knee_y pose.right_hip_x pose.right_hip_y;
			pose.left_knee_x pose.left_knee_y pose.left_hip_x pose.left_hip_y;
			pose.right_knee_x pose.right_knee_y pose.right_ankle_x pose.right_ankle_y;
			pose.left_knee_x pose.left_knee_y pose.left_ankle_x pose.left_ankle_y;
			]);
	J = step(shapeInserter, J, l);
	imwrite(J, [img_path(1:end-4) '_pose.jpg']);
end

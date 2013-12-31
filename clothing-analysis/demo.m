img_path = '8.png';
im = imread(img_path);
estimator = uci.PoseEstimator.default_estimator;
boxes = estimator.estimate(im);
pose = sbu.Pose(boxes);
pose.show('image', im);

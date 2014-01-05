function sleeve_type = sleeve_length_detector(sleeve_img_path)

    img = imread(sleeve_img_path);
    height = size(img,1);
    width = size(img,2);

    % Initialize the output images
    out = img;
    bin = zeros(height,width);
    % Convert the image from RGB to YCbCr
    img_ycbcr = rgb2ycbcr(img);
    Cb = img_ycbcr(:,:,2);
    Cr = img_ycbcr(:,:,3);

    % Detect Skin
    [r, c] = find(Cb>=77 & Cb<=127 & Cr>=133 & Cr<=173);
    numind = size(r, 1);
    ratio = numind/double(width*height)
    % Mark Skin Pixels
    for i=1:numind
        out(r(i),c(i),:) = [0 0 255];
        bin(r(i),c(i)) = 1;
    end
    imshow(img);
    figure; imshow(out);
    figure; imshow(bin);
    sleeve_type = 'unknown';
    % ratio may need to be adjust
    if (ratio > 0.66)
        sleeve_type = 'vest';
    else if (ratio > 0.2)
        sleeve_type = 'short sleeve';
    else
        sleeve_type = 'long sleeve';
    end
end

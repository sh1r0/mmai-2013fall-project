function segment_upper_part_ROI(img_path)

    im=imread(img_path);
    imgray = rgb2gray(im);
    BW_im = (imgray<253);
    seD = strel('diamond',1);
    BWfinal = imerode(BW_im,seD);
    imshow(BWfinal);
    [w,h] = size(BWfinal);
    roi_width = zeros(1,h);
    roi_margin_left = zeros(1, h);
    for i = 1:h
        roi_width(i) = sum(BWfinal(i,:));

        idx_list = find(BWfinal(i,:)==1);
        roi_margin_left(i) = idx_list(1);
    end
    bar(roi_width)
    collar_region = find_collar_region(roi_width, BWfinal)
    sleeve_region = find_sleeve_region(roi_width, roi_margin_left, BWfinal)

    %sleeve_region
    %main_region
    figure, imshow( im(collar_region.h_start:collar_region.h_end,collar_region.w_start:collar_region.w_end,:) )
    figure, imshow( im(sleeve_region.h_start:sleeve_region.h_end,sleeve_region.w_start:sleeve_region.w_end,:) )

    
end

function sleeve_region = find_sleeve_region(roi_width, roi_margin_left, BWfinal)
    sleeve_threshold = quantile(roi_width, .7)
    idx_list = find(roi_width >= sleeve_threshold)
    
    sleeve_region.h_start = idx_list(1)
    sleeve_region.h_end = idx_list(end)

    %idx_list = find(roi_width == max(roi_width))
    %h_widest = idx_list(1)
    sleeve_region.w_start = min(roi_margin_left)
    sleeve_region.w_end = sleeve_region.w_start + 130
end


% find collar region
function collar_region = find_collar_region(roi_width, BWfinal)
    
    idx_list = find(roi_width == min(roi_width))

    collar_region.h_start = idx_list(end)

    threshold = quantile(roi_width, .3)
    idx_list = find(roi_width >= threshold)
    collar_region.h_end = idx_list(1)

    idx_list1 = find(BWfinal(collar_region.h_start,:)==1)
    idx_list2 = find(BWfinal(collar_region.h_end,:)==1)
    collar_region.w_start = (idx_list1(1) + idx_list2(1))/2 
    collar_region.w_end = (idx_list1(end) + idx_list2(end))/2     
end
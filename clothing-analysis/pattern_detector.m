function pattern_type = pattern_detector(gabor_feature)
    addpath('../lib/libsvm-3.17/matlab/')
    load pattern_model
    %if (size(gabor_feature,1)==1)
    %    gabor_feature = gabor_feature'
    %end
    pattern_class = svmpredict(0, gabor_feature', pattern_model)
    pattern_type = 'unknown';

    if (pattern_class == 0)
        pattern_type='checked';
    else if(pattern_class == 1)
        pattern_type='striped';
    else (pattern_class == 2 )
        pattern_type='plain';
    end
end
classdef Segmentation < handle
    %SEGMENTATION segmentation structure
    properties
        map_path = '' % path to the saved map
        map           % map of superpixel indices (cached property)
        length        % size of superpixels
        labels        % labels for each superpixel
        features      % feature vectors for each superpixel
        pairs         % pairs of neighboring superpixels
        area          % area of each superpixel
    end
    
    properties (Constant)
        THRESH = 0.05 % threshold value used in Berkeley segmentation
    end
    
    methods
        %% OBJECT INITIALIZATION AND ACCESSORS
        function this = Segmentation( varargin )
            %SEGMENTATION
            %
            %    this = sbu.Segmentation(im, ...)
            %    this = sbu.Segmentation(...)
            %
            if nargin>0 && isnumeric(varargin{1})
                im = varargin{1};
                varargin = varargin(2:end);
                s = bsr.Segmentation(im);
                this.map = s.labels(this.THRESH);
            end
            if nargin>0, this.initialize(varargin{:}); end
        end
        
        function initialize( this, varargin )
            %INITIALIZE
            if isscalar(varargin) && isstruct(varargin{1})
                S = varargin{1};
            else
                S = struct(varargin{:});
            end
            props = intersect(properties(this),fieldnames(S));
            for i = 1:numel(props)
                this.(props{i}) = S.(props{i});
            end
        end
        
        function [ s ] = struct(this)
            %STRUCT convert to a struct array
            s = struct( 'map_path',{this.map_path},...
                        'map',     cell(1,numel(this)),...
                        'labels',  {this.labels},...
                        'features',{this.features});
            % load map when a storage file does not exist
            ind = arrayfun(@(x)~exist(x.map_path,'file'),s);
            [s(ind).map] = deal(this(ind).map);
        end
        
        function obj = copy(this)
            %COPY clone an instance
            obj = sbu.Segmentation(this.struct);
        end
        
        function x = get.map(this)
            %GET.MAP lazy loading of map data
            if isempty(this.map)
                this.map = this.load_map;
            end
            x = this.map;
        end
        
        function x = get.labels(this)
            %GET.LABELS
            if isempty(this.labels)
                this.labels = zeros(this.length,1,'uint32');
            end
            x = this.labels;
        end
        
        function x = get.length(this)
            %GET.LENGTH lazy calculation of superpixel size
            if isempty(this.length) || this.length==0
                this.length = numel(unique(this.map(:)));
            end
            x = this.length;
        end
        
        function x = get.pairs(this)
            %GET.PAIRS pair indices
            if ~isempty(this.map) && isempty(this.pairs)
                this.pairs = this.pairs_from_map();
            end
            x = this.pairs;
        end
        
        function [ a ] = get.area(this)
            %GET.AREA calculate areas of each segment
            if isempty(this.area) && ~isempty(this.map)
                this.area = full(sparse(double(this.map(:)),1,1));
            end
            a = this.area;
        end
        
        %% API
        function x = mask_of(this, id)
            %MASK_OF return logical mask of the segment
            x = (this.map == id);
        end
        
        function features_from(this, fmap, aggregator)
            %FEATURES_OF segmentation
            %  fmap:       M-by-N-by-D array of features.
            %  aggregator: Aggregation function applied to set of
            %              pixelwise features. The function should take
            %              row vectors of pixel representation and return
            %              a single row vector of the segment feature.
            %              (default: @(x) [mean(x),std(x)])
            if nargin < 3, aggregator = @(x) [mean(x),std(x)]; end
            
            % format fmap into row vectors
            fmap = reshape(fmap,[size(fmap,1)*size(fmap,2),size(fmap,3)]);
            % get segment features by aggregating a set of pixel features
            ind = accumarray(this.map(:),1:numel(this.map),...
                [this.length,1],@(x){x});
            for i = 1:numel(ind)
                %m = this.mask_of(uint32(i));
                m = ind{i};
                if iscell(aggregator)
                    x = fmap(m(:),:);
                    f = cell2mat(cellfun(@(j) aggregator{j}(x(:,j)),...
                            1:size(fmap,3), 'UniformOutput',false));
                else
                    f = aggregator(fmap(m(:),:));
                end
                if i==1 % allocate memory
                    this.features = zeros(this.length,numel(f));
                end
                this.features(i,:) = f;
            end
        end
        
        function [ L ] = label_map(this)
            %LABEL_MAP return map of labels
            L = uint16(this.labels(this.map));
        end
        
        function [ ind ] = is_overlapped(this, bbox, varargin)
            %IS_OVERLAPPED return trimap given bbox
            
            thresh = 0.75;
            for i = 1:2:numel(varargin)
                switch varargin{i}
                    case 'thresh', thresh = varargin{i+1};
                end
            end
            
            % get roi
            x0 = max(1,bbox(1));
            y0 = max(1,bbox(2));
            x1 = min(size(this.map,2),bbox(1)+bbox(3)-1);
            y1 = min(size(this.map,1),bbox(2)+bbox(4)-1);
            roi_map = this.map(y0:y1,x0:x1);
            
            % threshold on overlapping ratio for each
            segment_ids = unique(roi_map(:));
            ind = false(this.length,1);
            for i = 1:numel(segment_ids)
                segment_area = nnz(this.map(:)==segment_ids(i));
                roi_area = nnz(roi_map(:)==segment_ids(i));
                ind(segment_ids(i)) = roi_area / segment_area > thresh;
            end
        end
        
        %% Utility
        function [ I ] = show_superpixels(this, im, varargin)
            %SHOW_SUPERPIXELS
            if nargin == 2
                % When given an image
                edge_color = [255,0,0];
                m = this.map;
                e = (m ~= [m(2:end,:);m(end,:)]) |...
                    (m ~= [m(1,:);m(1:end-1,:)]) |...
                    (m ~= [m(:,2:end),m(:,end)]) |...
                    (m ~= [m(:,1),m(:,1:end-1)]);
                for i = 1:size(im,3)
                    I = im(:,:,i);
                    I(e) = edge_color(i);
                    im(:,:,i) = I;
                end
                I = im;
                h = imshow(I);
            else
                % When nothing is given
                I = label2rgb(this.map);
                h = imshow(I);
            end
        end
        
        function [ I ] = show(this, varargin)
            %SHOW show and return 2D array of labels
            
            % Options
            I = [];
            clothings = [];
            show_colorbar = true;
            ALPHA = .67;
            for i = 1:2:numel(varargin)
                switch varargin{i}
                    case 'image', I = varargin{i+1};
                    case 'labels', clothings = varargin{i+1};
                    case 'alpha',  ALPHA = varargin{i+1};
                    case 'colorbar', show_colorbar = varargin{i+1};
                end
            end
            if ischar(I), I = imread(I); end
            L = this.labels;
            
            % Create index
            labels_ = unique(L(:));
            L = arrayfun(@(l) find(labels_==l),L);
            L = L(this.map);
            cmap = [ones(1,3);hsv(numel(labels_)-1)];
            
            % Render
            if isempty(I)
                imshow(L,cmap);
            else
                imshow(I);
                hold on; h = imshow(L,cmap); hold off;
                set(h,'AlphaData',ones(size(L))*ALPHA);
            end
            % add colorbar
            if ~isempty(clothings)
                names = arrayfun(@(i)clothings([clothings.id]==labels_(i)),...
                    1:numel(labels_),'UniformOutput',false);
                ind = cellfun(@isempty,names);
                names(ind) = repmat({'null'},1,nnz(ind));
                names(~ind) = cellfun(@(x) x.name,names(~ind),...
                    'UniformOutput',false);
                if show_colorbar
                    colorbar('TickLength',[0,0],...
                             'YTick',0:numel(labels_)-1,...
                             'YTickLabel',names,...
                             'YTickMode','manual',...
                             'YTickLabelMode','manual');
                end
            end
            if nargout > 0
                drawnow;
                g = getframe(gca);
                I = g.cdata(1:end-1,1:end-1,:);
            end
        end
        
        function c = channel_map(this, x)
            %CHANNEL_MAP convert a map into uint8 format so that the map
            %can be saved as an image
            if nargin < 2, x = this.map; end
            msk = uint32(255);
            c = uint8(cat(3,...
                bitand(x,msk),...
                bitand(bitshift(x,-8),msk),...
                bitand(bitshift(x,-16),msk)));
        end
        
        function x = dechannel_map(this, c)
            %DECHANNEL_MAP convert an uint8 format to uint32 format
            if nargin < 2, c = imread(this.map_path); end
            c = uint32(c);
            % Dechannel on load
            x = bitor(bitor(c(:,:,1),...
                bitshift(c(:,:,2),8)),...
                bitshift(c(:,:,3),16));
        end
        
        function save_map(this, file_path)
            %SAVE_MAP save a map file to a specified path
            if nargin < 2, file_path = this.map_path; end
            imwrite(this.channel_map,file_path);
            this.map_path = file_path;
        end
        
        function m = load_map(this, file_path)
            %LOAD_MAP load a map file from a specified path
            if nargin > 1, this.map_path = file_path; end
            m = [];
            if ischar(this.map_path) && exist(this.map_path,'file')
                m = this.dechannel_map;
            end
        end
    end
    
    %% Internal
    methods (Access = private)
        function [ s ] = pairs_from_map( this )
            %PAIRS_FROM_MAP find neighboring pairs of segment ids
            
            % get boundary index
            m = this.map;
            shift.u = (m ~= [m(2:end,:);m(end,:)]);
            shift.d = (m ~= [m(1,:);m(1:end-1,:)]);
            shift.l = (m ~= [m(:,2:end),m(:,end)]);
            shift.r = (m ~= [m(:,1),m(:,1:end-1)]);
            % get neighboring segment ids in row vectors
            s = [m(shift.u),m(shift.d);m(shift.l),m(shift.r)];
            s = unique(s,'rows');
            s = unique([s;fliplr(s)],'rows');
            s(s(:,1)>s(:,2),:) = [];
        end
    end
    
    %% Helpers
    methods (Hidden)
        function obj = saveobj(this)
            %SAVEOBJ callback on save
            obj = this.struct;
        end
    end
    
    methods (Static)
        function [ P ] = color_palette(L, clothings)
            %COLOR_PALETTE return color palette
            labels_ = unique(L(:));
            cmap = [ones(1,3);hsv(numel(labels_)-1)];
            P = struct('r',num2cell(cmap(:,1)),...
                       'g',num2cell(cmap(:,2)),...
                       'b',num2cell(cmap(:,3)));
            if nargin > 1
                names = arrayfun(@(i)clothings([clothings.id]==labels_(i)),...
                    1:numel(labels_),'UniformOutput',false);
                ind = cellfun(@isempty,names);
                names(ind) = repmat({'null'},1,nnz(ind));
                names(~ind) = cellfun(@(x) x.name,names(~ind),...
                    'UniformOutput',false);
                if isempty(names), names = {'null'}; end
                [P.name] = deal(names{:});
            end
        end
    end
    
    methods (Hidden, Static)
        function [ this ] = loadobj(obj)
            %LOADOBJ callback on load
            this = sbu.Segmentation(obj);
        end
    end
    
end


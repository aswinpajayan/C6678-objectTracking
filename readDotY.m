%% code for reading .y images
% TI provided .y images as a part of their workshop held in college.
% We can read those .y images using these three simple lines of code

fp = fopen('./input_ball.y','rb');
x = fread(fp,"uint8");
img = reshape(x,256,256); %% arg[2] = width of image; arg[3] = height of image
imagesc(img); 

%% similarly you can use the following code to create a .y image

img = imread('./ball.png'); %assuming gray scale image
s = size(img)
x = reshape(img,1,s(1).*s(2));
fp = fopen('./ball_out.y','w');
fwrite(fp,x,"uint8");
fclose(fp);

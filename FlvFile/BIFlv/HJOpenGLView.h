//
//  HJOpenGLView.m
//  Smart_Device_Client
//
//  Created by Josie on 2017/9/22.
//  Copyright © 2017年 Josie. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#define SCREENWIDTH [UIScreen mainScreen].bounds.size.width
#define SCREENHEIGHT [UIScreen mainScreen].bounds.size.height

#define PLAYVIEW_PORTRAIT_STARTX    0
#define PLAYVIEW_PORTRAIT_STARTY    (SCREENHEIGHT - SCREENWIDTH * (480 / 640.0)) / 2.0
#define PLAYVIEW_PORTRAIT_WIDTH     SCREENWIDTH
#define PLAYVIEW_PORTRAIT_HEIGHT    SCREENWIDTH * (480 / 640.0)

@interface HJOpenGLView : UIView

- (void)setupGL;
- (void)displayPixelBuffer:(CVPixelBufferRef)pixelBuffer;

@end

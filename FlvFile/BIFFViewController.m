//
//  BIFFViewController.m
//  FlvFile
//  FFmpeg  解码
//  Created by kirk on 2019/1/14.
//  Copyright © 2019 kirk. All rights reserved.
//

#import "BIFFViewController.h"
#include "avformat.h"
#include "avcodec.h"
#include "BIMalloc.h"
#import "HJOpenGLView.h"

@interface BIFFViewController ()
@property (weak, nonatomic) IBOutlet UIButton* bt;
@property (nonatomic, strong) HJOpenGLView  *playView;
@end

@implementation BIFFViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    [self.view addSubview:self.playView];
    self.playView.frame = CGRectMake(PLAYVIEW_PORTRAIT_STARTX, PLAYVIEW_PORTRAIT_STARTY, PLAYVIEW_PORTRAIT_WIDTH, PLAYVIEW_PORTRAIT_HEIGHT);
    [self.playView setupGL];
    [self.view bringSubviewToFront:self.bt];
}

- (IBAction)btClicked:(id)sender {
    av_register_all();
//    avcodec_register_all();
    AVFormatContext* formatContext = avformat_alloc_context();
    
    NSString* filePath = [[NSBundle mainBundle] pathForResource:@"cuc_ieschool" ofType:@"flv"];
    
    const char* url =  [filePath cStringUsingEncoding:(NSUTF8StringEncoding)];
    
    AVInputFormat* inputFormat = av_find_input_format("flv");
    
    int status = avformat_open_input(&formatContext,url, inputFormat, 0);
    NSLog(@"%d",status);
    status = avformat_find_stream_info(formatContext, NULL);
    NSLog(@"%d",status);
    
    if (formatContext->streams == NULL) {
        return;
    }
    
    AVStream* stream = formatContext->streams[0];
    
    AVCodec* decodec = avcodec_find_decoder(stream->codecpar->codec_id);
    AVCodecContext* codecContext = avcodec_alloc_context3(decodec);
    avcodec_parameters_to_context(codecContext, stream->codecpar);
    av_codec_set_pkt_timebase(codecContext, stream->time_base);
    
    
    status = avcodec_open2(codecContext, decodec, NULL);
    NSLog(@"%d",status);
    AVPacket* package = bi_malloc(sizeof(AVPacket));
    AVFrame* frame = bi_malloc(sizeof(AVFrame));
    
    status = av_read_frame(formatContext, package);
    NSLog(@"%d",status);
    while (status == 0) {
        status = avcodec_send_packet(codecContext, package);
        NSLog(@"%d",status);
        if (package->stream_index == 0) {
            status = avcodec_receive_frame(codecContext, frame);
//            NSLog(@"%d",status);
            if (status == 0) {
                [self dispatchAVFrame:frame];
            }
        }
        status = av_read_frame(formatContext, package);
//        NSLog(@"%d",status);
    }
    
    bi_free(package);
    bi_free(frame);
    NSLog(@"$$  total_alloc_size:%zu",total_alloc_size);
    
    avformat_free_context(formatContext);
    avcodec_free_context(&codecContext);
}


- (void)dispatchAVFrame:(AVFrame*) frame{
    if(!frame || !frame->data[0]){
        return;
    }
    
    CVReturn theError;
//    if (!self.pixelBufferPool){
    CVPixelBufferPoolRef _pixelBufferPool;
        NSMutableDictionary* attributes = [NSMutableDictionary dictionary];
        [attributes setObject:[NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange] forKey:(NSString*)kCVPixelBufferPixelFormatTypeKey];
        [attributes setObject:[NSNumber numberWithInt:frame->width] forKey: (NSString*)kCVPixelBufferWidthKey];
        [attributes setObject:[NSNumber numberWithInt:frame->height] forKey: (NSString*)kCVPixelBufferHeightKey];
        [attributes setObject:@(frame->linesize[0]) forKey:(NSString*)kCVPixelBufferBytesPerRowAlignmentKey];
        [attributes setObject:[NSDictionary dictionary] forKey:(NSString*)kCVPixelBufferIOSurfacePropertiesKey];
        theError = CVPixelBufferPoolCreate(kCFAllocatorDefault, NULL, (__bridge CFDictionaryRef) attributes, &_pixelBufferPool);
        if (theError != kCVReturnSuccess){
            NSLog(@"CVPixelBufferPoolCreate Failed");
        }
//    }
    
    CVPixelBufferRef pixelBuffer = nil;
    theError = CVPixelBufferPoolCreatePixelBuffer(NULL, _pixelBufferPool, &pixelBuffer);
    if(theError != kCVReturnSuccess){
        NSLog(@"CVPixelBufferPoolCreatePixelBuffer Failed");
    }else{
        
        CVPixelBufferLockBaseAddress(pixelBuffer, 0);
        size_t bytePerRowY = CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0);
        size_t bytesPerRowUV = CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 1);
        void* base = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
        memcpy(base, frame->data[0], bytePerRowY * frame->height);
        base = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1);
        memcpy(base, frame->data[1], bytesPerRowUV * frame->height/2);
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
        
        [self dispatchPixelBuffer:pixelBuffer];
        CVPixelBufferRelease(pixelBuffer);
    }
}

-(void)dispatchPixelBuffer:(CVPixelBufferRef)bufferRef{
    [self.playView displayPixelBuffer:bufferRef];
    
}

-(HJOpenGLView *)playView
{
    if (!_playView) {
        _playView = [[HJOpenGLView alloc] init];
    }
    return _playView;
}
@end

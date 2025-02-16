





















#import <Foundation/Foundation.h>
#import <vlc_player.h>

NS_ASSUME_NONNULL_BEGIN

@class VLCInputStats;
@class VLCTrackMetaData;
@class VLCProgramMetaData;
@class VLCInputItem;

extern NSString *VLCPlayerElementaryStreamID;
extern NSString *VLCTick;





extern NSString *VLCPlayerCurrentMediaItemChanged;






extern NSString *VLCPlayerMetadataChangedForCurrentMedia;





extern NSString *VLCPlayerStateChanged;





extern NSString *VLCPlayerErrorChanged;

extern NSString *VLCPlayerBufferFill;





extern NSString *VLCPlayerBufferChanged;





extern NSString *VLCPlayerRateChanged;






extern NSString *VLCPlayerCapabilitiesChanged;





extern NSString *VLCPlayerTimeAndPositionChanged;





extern NSString *VLCPlayerLengthChanged;





extern NSString *VLCPlayerTitleSelectionChanged;





extern NSString *VLCPlayerTitleListChanged;





extern NSString *VLCPlayerChapterSelectionChanged;





extern NSString *VLCPlayerProgramSelectionChanged;





extern NSString *VLCPlayerProgramListChanged;





extern NSString *VLCPlayerABLoopStateChanged;





extern NSString *VLCPlayerTeletextMenuAvailable;





extern NSString *VLCPlayerTeletextEnabled;





extern NSString *VLCPlayerTeletextPageChanged;





extern NSString *VLCPlayerTeletextTransparencyChanged;





extern NSString *VLCPlayerAudioDelayChanged;





extern NSString *VLCPlayerSubtitlesDelayChanged;






extern NSString *VLCPlayerDelayChangedForSpecificElementaryStream;





extern NSString *VLCPlayerSubtitlesFPSChanged;





extern NSString *VLCPlayerRecordingChanged;





extern NSString *VLCPlayerRendererChanged;

extern NSString *VLCPlayerInputStats;





extern NSString *VLCPlayerStatisticsUpdated;





extern NSString *VLCPlayerTrackListChanged;





extern NSString *VLCPlayerTrackSelectionChanged;





extern NSString *VLCPlayerFullscreenChanged;





extern NSString *VLCPlayerListOfVideoOutputThreadsChanged;





extern NSString *VLCPlayerWallpaperModeChanged;





extern NSString *VLCPlayerVolumeChanged;





extern NSString *VLCPlayerMuteChanged;

extern const CGFloat VLCVolumeMaximum;
extern const CGFloat VLCVolumeDefault;

@interface VLCPlayerController : NSObject

- (instancetype)initWithPlayer:(vlc_player_t *)player;





- (int)start;





- (void)startInPausedState:(BOOL)startPaused;




- (void)pause;




- (void)resume;




- (void)togglePlayPause;




- (void)stop;






@property (readonly) enum vlc_player_abloop abLoopState;







- (int)setABLoop;





- (int)disableABLoop;






@property (readwrite, nonatomic) enum vlc_player_media_stopped_action actionAfterStop;





- (void)nextVideoFrame;






@property (readonly, nullable) VLCInputItem * currentMedia;





- (int)setCurrentMedia:(VLCInputItem *)currentMedia;




@property (readonly) vlc_tick_t durationOfCurrentMediaItem;




@property (readonly, copy, nullable) NSURL *URLOfCurrentMediaItem;




@property (readonly, copy, nullable) NSString *nameOfCurrentMediaItem;






@property (readonly) enum vlc_player_state playerState;






@property (readonly) enum vlc_player_error error;






@property (readonly) float bufferFill;






@property (readwrite, nonatomic) float playbackRate;




- (void)incrementPlaybackRate;



- (void)decrementPlaybackRate;






@property (readonly) BOOL seekable;






@property (readonly) BOOL rewindable;






@property (readonly) BOOL pausable;






@property (readonly) BOOL recordable;






@property (readonly) BOOL rateChangable;








@property (readonly) vlc_tick_t time;






- (void)setTimeFast:(vlc_tick_t)time;






- (void)setTimePrecise:(vlc_tick_t)time;








@property (readonly) float position;






- (void)setPositionFast:(float)position;






- (void)setPositionPrecise:(float)position;





- (void)displayPosition;




- (void)jumpForwardExtraShort;




- (void)jumpBackwardExtraShort;




- (void)jumpForwardShort;




- (void)jumpBackwardShort;




- (void)jumpForwardMedium;




- (void)jumpBackwardMedium;




- (void)jumpForwardLong;




- (void)jumpBackwardLong;








@property (readonly) vlc_tick_t length;





@property (readwrite, nonatomic) size_t selectedTitleIndex;





- (const struct vlc_player_title * _Nullable)selectedTitle;





@property (readonly) size_t numberOfTitlesOfCurrentMedia;





- (const struct vlc_player_title *)titleAtIndexForCurrentMedia:(size_t)index;





@property (readwrite, nonatomic) size_t selectedChapterIndex;




- (void)selectNextChapter;




- (void)selectPreviousChapter;




@property (readonly) size_t numberOfChaptersForCurrentTitle;




- (nullable const struct vlc_player_chapter *)chapterAtIndexForCurrentTitle:(size_t)index;






@property (readonly) int selectedProgramID;





- (void)selectProgram:(VLCProgramMetaData *)program;





@property (readonly) size_t numberOfPrograms;




- (nullable VLCProgramMetaData *)programAtIndex:(size_t)index;




- (nullable VLCProgramMetaData *)programForID:(int)programID;





@property (readonly) BOOL teletextMenuAvailable;





@property (readwrite, nonatomic) BOOL teletextEnabled;











@property (readwrite, nonatomic) unsigned int teletextPage;






@property (readwrite, nonatomic) BOOL teletextTransparent;







@property (readwrite, nonatomic) vlc_tick_t audioDelay;







@property (readwrite, nonatomic) vlc_tick_t subtitlesDelay;





- (vlc_tick_t)delayForElementaryStreamID:(vlc_es_id_t *)esID;









- (int)setDelay:(vlc_tick_t)delay forElementaryStreamID:(vlc_es_id_t *)esID relativeWhence:(BOOL)relative;






@property (readwrite, nonatomic) float subtitlesFPS;





@property (readwrite, nonatomic) unsigned int subtitleTextScalingFactor;





@property (readwrite, nonatomic) BOOL enableRecording;




- (void)toggleRecord;










- (int)addAssociatedMediaToCurrentFromURL:(NSURL *)URL
ofCategory:(enum es_format_category_e)category
shallSelectTrack:(BOOL)selectTrack
shallDisplayOSD:(BOOL)showOSD
shallVerifyExtension:(BOOL)verifyExtension;






@property (readwrite, nonatomic, nullable) vlc_renderer_item_t *rendererItem;




- (void)navigateInInteractiveContent:(enum vlc_player_nav)navigationAction;






@property (readonly) VLCInputStats *statistics;

#pragma mark - track selection








- (void)selectTrack:(VLCTrackMetaData *)track exclusively:(BOOL)exclusiveSelection;






- (void)unselectTrack:(VLCTrackMetaData *)track;






- (void)unselectTracksFromCategory:(enum es_format_category_e)category;






- (void)selectPreviousTrackForCategory:(enum es_format_category_e)category;






- (void)selectNextTrackForCategory:(enum es_format_category_e)category;





@property (readonly, nullable) NSArray<VLCTrackMetaData *>* audioTracks;





@property (readonly, nullable) NSArray<VLCTrackMetaData *>* videoTracks;





@property (readonly, nullable) NSArray<VLCTrackMetaData *>* subtitleTracks;

#pragma mark - video output properties







@property (readonly, nullable) vout_thread_t *mainVideoOutputThread;






@property (readonly, nullable) vout_thread_t *videoOutputThreadForKeyWindow;






@property (readonly, nullable, copy) NSArray<NSValue *> *allVideoOutputThreads;





@property (readwrite, nonatomic) BOOL fullscreen;




@property (readwrite) BOOL activeVideoPlayback;




- (void)toggleFullscreen;





@property (readwrite, nonatomic) BOOL wallpaperMode;




- (void)takeSnapshot;




- (void)displayOSDMessage:(NSString *)message;




@property (nonatomic, readwrite) BOOL aspectRatioIsLocked;

#pragma mark - audio output properties






@property (readwrite, nonatomic) float volume;




- (void)incrementVolume;




- (void)decrementVolume;





@property (readwrite, nonatomic) BOOL mute;




- (void)toggleMute;






@property (readonly, nullable) audio_output_t *mainAudioOutput;

- (int)enableAudioFilterWithName:(NSString *)name state:(BOOL)state;

@end

@interface VLCInputStats : NSObject

- (instancetype)initWithStatsStructure:(const struct input_stats_t *)stats;


@property (readonly) int64_t inputReadPackets;
@property (readonly) int64_t inputReadBytes;
@property (readonly) float inputBitrate;


@property (readonly) int64_t demuxReadPackets;
@property (readonly) int64_t demuxReadBytes;
@property (readonly) float demuxBitrate;
@property (readonly) int64_t demuxCorrupted;
@property (readonly) int64_t demuxDiscontinuity;


@property (readonly) int64_t decodedAudio;
@property (readonly) int64_t decodedVideo;


@property (readonly) int64_t displayedPictures;
@property (readonly) int64_t lostPictures;


@property (readonly) int64_t playedAudioBuffers;
@property (readonly) int64_t lostAudioBuffers;

@end

@interface VLCTrackMetaData : NSObject

- (instancetype)initWithTrackStructure:(const struct vlc_player_track *)track;

@property (readonly) vlc_es_id_t *esID;
@property (readonly) NSString *name;
@property (readonly) BOOL selected;

@end

@interface VLCProgramMetaData : NSObject

- (instancetype)initWithProgramStructure:(const struct vlc_player_program *)structure;

@property (readonly) int group_id;
@property (readonly) NSString *name;
@property (readonly) BOOL selected;
@property (readonly) BOOL scrambled;

@end

NS_ASSUME_NONNULL_END

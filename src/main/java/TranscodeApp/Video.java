package TranscodeApp;

import javafx.scene.media.Media;

public class Video {

    private Media videoMedia;
    private String path;
    private final String name;
    private long sizeInBytes;

    public String getName() {
        return name;
    }

    public Video(Media videoMedia, String path, long sizeInBytes, String name) {
        this.videoMedia = videoMedia;
        this.path = path;
        this.sizeInBytes = sizeInBytes;
        this.name=name;
    }

    public Media getVideoMedia() {
        return videoMedia;
    }

    public void setVideoMedia(Media videoMedia) {
        this.videoMedia = videoMedia;
    }

    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public long getSizeInBytes() {
        return sizeInBytes;
    }

    public void setSizeInMegabytes(long sizeInBytes) {
        this.sizeInBytes = sizeInBytes;
    }
}

package TranscodeApp;

import javafx.scene.media.Media;

import java.io.File;
import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.*;

public class VideoCollector {

    private final Set<Video> videoSet=new HashSet<>();
    long sumOfFilesSizes=0;

    public Set<Video> getVideoSet() {
        return videoSet;
    }

    public int getCountOfVideos(){
        return videoSet.size();
    }

    public long getSumOfFilesSizes(){
        return sumOfFilesSizes;
    }

    public String getReadableSumOfFilesSizes(){
        double sumInMB= sumOfFilesSizes/(1024.0*1024.0);
        if(sumInMB >= 1024.0){
            double sumInGb=sumInMB/1024.0;
            BigDecimal sumInBigDecimal = BigDecimal.valueOf(sumInGb);
            sumInBigDecimal= sumInBigDecimal.setScale(2, RoundingMode.HALF_UP);
            return String.valueOf(sumInBigDecimal.doubleValue()+" GB");

        }else{
            BigDecimal sumInBigDecimal = BigDecimal.valueOf(sumInMB);
            sumInBigDecimal= sumInBigDecimal.setScale(2, RoundingMode.HALF_UP);
            return String.valueOf(sumInBigDecimal.doubleValue()+" MB");
        }

    }

    public void addVideoToSet(Video newVideo){
        sumOfFilesSizes+=newVideo.getSizeInBytes();
        videoSet.add(newVideo);
    }

    //TO DO: Wyszukanie video z videoSet za pomocą strumieni
    public Video getVideoByFilePath(String filePath){
    Iterator<Video> iterator=videoSet.iterator();
    while(iterator.hasNext()){
        Video tempVideo= iterator.next();
        if(tempVideo.getPath()==filePath){
            return tempVideo;
        }
    }
    return null;
    }


    public void removeVideoFromSet(String filePath){


        videoSet.forEach(v-> {
            if(v.getPath().equals(filePath)){
                sumOfFilesSizes-=v.getSizeInBytes();
                videoSet.remove(v);
                return;
            }
        });

    }


}

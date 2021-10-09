package TranscodeApp;

import java.io.File;
import java.io.IOException;
import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.ArrayList;
import java.util.List;


public class TranscodeManager {


    String outputDirectory=null;
    VideoCollector videoCollectorInTranscodeManager;
    List<Observer> observers;



    private Long compressedSize= 0L;
    public TranscodeManager(){
        observers=new ArrayList<>();
    }

    public String getOutputDirectory() {
        return outputDirectory;
    }

    public void addObserver(Observer observer){
        this.observers.add(observer);
    }

    public void setOutputDirectory(String outputDirectory) {
        this.outputDirectory = outputDirectory;
    }

    public void giveVideoCollectorToTranscoder(VideoCollector videoCollector){
        this.videoCollectorInTranscodeManager=videoCollector;
    }

    public void setupTranscode(int crf){
    //Transcoder transcoder=new Transcoder();
    //transcoder.transcode("C:\\\\Users\\\\mswar\\\\Desktop\\\\ENIGMEN.mp4","ouuut.mp4","medium","20");

        Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                videoCollectorInTranscodeManager.getVideoSet().forEach(video -> {
                    System.out.println("Input: "+video.getPath()+"; Output: "+outputDirectory+"\\"+video.getName());
                    Transcoder transcoder=new Transcoder();
                    transcoder.transcode(video.getPath(),outputDirectory+"\\"+video.getName(),"slow","21");

                    File file = new File(outputDirectory+"\\"+video.getName());
                    compressedSize+=file.length();
                    for(Observer observer : observers){
                        observer.updateProgress();
                    }
            });
                for(Observer observer : observers){
                    try {
                        observer.finish();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
        };

//        videoCollectorInTranscodeManager.getVideoSet().forEach(video -> {
//            System.out.println("Input: "+video.getPath()+"; Output: "+outputDirectory+"\\"+video.getName());
//            Transcoder transcoder=new Transcoder();
//                transcoder.transcode(video.getPath(),outputDirectory+"\\"+video.getName(),"medium","20");
//                for(Observer observer : observers){
//                    observer.updateProgress();
//                }
//            Thread thread = new Thread(new Runnable() {
//
//                @Override
//                public void run() {
//                    Transcoder transcoder=new Transcoder();
//                transcoder.transcode(video.getPath(),outputDirectory+"\\"+video.getName(),"fast","20");
//                for(Observer observer : observers){
//                    observer.updateProgress();
//                }
//                }
//            });
//        thread.start();
        });
        thread.start();
    }
    public String compareCompressedSize(long inputSum){
        long differenceSize = inputSum-compressedSize;
        double sumInMB= differenceSize/(1024.0*1024.0);
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

}

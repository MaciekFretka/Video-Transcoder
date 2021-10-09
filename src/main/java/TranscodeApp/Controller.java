package TranscodeApp;

import javafx.animation.KeyFrame;
import javafx.animation.KeyValue;
import javafx.animation.Timeline;
import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Node;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.media.Media;
import javafx.scene.media.MediaException;
import javafx.scene.media.MediaPlayer;
import javafx.scene.media.MediaView;
import javafx.stage.DirectoryChooser;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import javafx.util.Duration;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.Locale;
import java.util.ResourceBundle;

public class Controller implements Initializable,Observer {

    @FXML
    public Button addVideoButton,deleteVideoButton,playButton,pauseButton,convertButton,aboutButton;

    @FXML
    public MediaView mediaView;

    @FXML
    public Label sizeLabel,finishLabel;

    @FXML
    public ListView <String> listView=new ListView<>();
    ObservableList<String> items =FXCollections.observableArrayList ();

    @FXML
    public ProgressBar progressBar;

    @FXML
    public ProgressIndicator progressIndicator;

    private Stage stage;
    private Scene scene;
    Parent root;

    private TranscodeManager transcodeManager;
    private VideoCollector videoCollector;
    private MediaPlayer mediaPlayer;
    private boolean isVideoPlayingFlag=false;

    private double progress =0.0;

    String language;
    String country;
    Locale locale;
    ResourceBundle resourceBundle;

    String alertTitle,existingFileAlertHeader,existingFileAlertContent,fileFormatAlertHeader,fileFormatAlertContent,
            fileChooserTitle,directoryChooserTitle,finishText;
    @Override
    public void initialize(URL location, ResourceBundle resources) {
        language = "pl";
        country = "PL";
        locale = new Locale(language,country);
        resourceBundle=ResourceBundle.getBundle("LanguageBundle",locale);
        setTexts();


        videoCollector=new VideoCollector();
        listView.setItems(items);


    }

    public void setEnglish(){


        language = "en";
        country = "EN";
        locale = new Locale(language,country);
        resourceBundle=ResourceBundle.getBundle("LanguageBundle",locale);
        setTexts();


    }
    public void setPolish(){

        language = "pl";
        country = "PL";
        locale = new Locale(language,country);
        resourceBundle=ResourceBundle.getBundle("LanguageBundle",locale);
        setTexts();
    }

    public void setTexts(){
        playButton.setText(resourceBundle.getString("playButton"));
        pauseButton.setText(resourceBundle.getString("pauseButton"));
        convertButton.setText(resourceBundle.getString("convertButton"));
        addVideoButton.setText(resourceBundle.getString("addVideoButton"));
        deleteVideoButton.setText(resourceBundle.getString("deleteVideoButton"));
        alertTitle= resourceBundle.getString("alertTitle");
        existingFileAlertHeader=resourceBundle.getString("existingFileAlertHeader");
        existingFileAlertContent=resourceBundle.getString("existingFileAlertContent");
        fileFormatAlertHeader=resourceBundle.getString("fileFormatAlertHeader");
        fileFormatAlertContent=resourceBundle.getString("fileFormatAlertContent");
        fileChooserTitle=resourceBundle.getString("fileChooserTitle");
        directoryChooserTitle=resourceBundle.getString("directoryChooserTitle");
        finishText=resourceBundle.getString("finishText");


    }


    public void addVideo(){
        File selectedVideo = chooseFile();
        Media newVideo = null;
        try{
            if(items.contains(selectedVideo.getAbsolutePath())) {
                existingFileAlert();
                return;
            }
            newVideo = new Media(selectedVideo.toURI().toString());
            Video video=new Video(newVideo,selectedVideo.getAbsolutePath(),selectedVideo.length(),selectedVideo.getName());
            videoCollector.addVideoToSet(video);

            items.add(selectedVideo.getAbsolutePath());
            sizeLabel.setText(videoCollector.getReadableSumOfFilesSizes());


        }catch (MediaException e){
            fileFormatErrorLog();
        }



    }
    private void existingFileAlert(){
        Alert alert = new Alert(Alert.AlertType.ERROR);

        alert.setTitle(alertTitle);
        alert.setHeaderText(existingFileAlertHeader);
        alert.setContentText(existingFileAlertContent);

        alert.showAndWait();
    }
    private void fileFormatErrorLog() {
        Alert alert = new Alert(Alert.AlertType.ERROR);
        alert.setTitle(alertTitle);
        alert.setHeaderText(fileFormatAlertHeader);
        alert.setContentText(fileFormatAlertContent);

        alert.showAndWait();
    }

    private File chooseFile (){
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle(fileChooserTitle);
        File selectedFile = new File(String.valueOf(fileChooser.showOpenDialog(null)));
        return selectedFile;
    }

    public void playVideo(){
        if(isVideoPlayingFlag){
            mediaPlayer.pause();
        }
        Video videoToPlay=getSelectedVideoFromCollector();
        mediaPlayer=new MediaPlayer(videoToPlay.getVideoMedia());
        mediaView.setMediaPlayer(mediaPlayer);
        mediaPlayer.play();
        isVideoPlayingFlag=true;
    }

    private Video getSelectedVideoFromCollector(){
        String path=listView.getSelectionModel().getSelectedItem().toString();
        return videoCollector.getVideoByFilePath(path);
    }


    public void pauseVideo(){
        mediaPlayer.pause();
        isVideoPlayingFlag=false;
    }

    public void removeVideo(){
        String selectedPath = listView.getSelectionModel().getSelectedItem().toString();
        videoCollector.removeVideoFromSet(selectedPath);
        items.remove(selectedPath);
        sizeLabel.setText(videoCollector.getReadableSumOfFilesSizes());
        pauseVideo();
    }
    public void sayHi(){
        String selectedPath = listView.getSelectionModel().getSelectedItem().toString();
        System.out.println(selectedPath);
    }


    public void transcodeAction(){
        Timeline task = new Timeline(
                new KeyFrame(
                        Duration.ZERO,
                        new KeyValue(progressBar.progressProperty(), 0)
                ),
                new KeyFrame(
                        Duration.seconds(2),
                        new KeyValue(progressBar.progressProperty(), 1)
                )
        );

        transcodeManager =new TranscodeManager();
        setOutputFolder();
        if(!transcodeManager.getOutputDirectory().equals(null)){
            System.out.println(transcodeManager.getOutputDirectory());

            transcodeManager.addObserver(this);
            transcodeManager.giveVideoCollectorToTranscoder(videoCollector);
            transcodeManager.setupTranscode(21);

        }
        progressIndicator.setVisible(true);

    }
    public void setOutputFolder(){

        DirectoryChooser directoryChooser = new DirectoryChooser();
        directoryChooser.setTitle(directoryChooserTitle);
        String outputDirectoryPath=String.valueOf(directoryChooser.showDialog(null));
        transcodeManager.setOutputDirectory(outputDirectoryPath);



    }

    @Override
    public void updateProgress() {

        progress+= 1.0/(double)videoCollector.getCountOfVideos();
        progressBar.setProgress(progress);
    System.out.println(progress);

      //  System.out.println("Gratulacje! Zaoszczędziłeś "+transcodeManager.compareCompressedSize(videoCollector.getSumOfFilesSizes()));
    }

    public void switchScene(ActionEvent event) throws IOException {
        Platform.runLater(new Runnable() {
            @Override
            public void run() {
                root = null;
                try {
                    FXMLLoader loader = new FXMLLoader(getClass().getResource("/finishScene.fxml"));
                    root =loader.load();

                    ControllerFinish controllerFinish = loader.getController();
                    controllerFinish.setFinishText(finishText+transcodeManager.compareCompressedSize(videoCollector.getSumOfFilesSizes()));

                } catch (IOException e) {
                    e.printStackTrace();
                }
                stage = (Stage)sizeLabel.getScene().getWindow();
                scene = new Scene(root);

                String cssPath = this.getClass().getResource("/application.css").toExternalForm();
                scene.getStylesheets().add(cssPath);
                stage.setResizable(false);
                stage.setScene(scene);
                stage.show();


              //  finishLabel.setText("Gratulacje!"+"Zaoszczędziłeś: "+transcodeManager.compareCompressedSize(videoCollector.getSumOfFilesSizes()));
            }
        });

    }

    @Override
    public void finish() throws IOException {

        this.switchScene(new ActionEvent());
    }

}

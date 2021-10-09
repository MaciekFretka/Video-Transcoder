package TranscodeApp;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.scene.layout.AnchorPane;
import javafx.stage.Stage;

import java.net.URL;
import java.util.Objects;

public class Main extends Application {

    public static void main(String[] args) {
        launch();
    }
    @Override
    public void start(Stage stage) throws Exception {
        AnchorPane mainPane = FXMLLoader.load(getClass().getResource("/MainView.fxml"));
        Scene scene = new Scene(mainPane);
        String cssPath = this.getClass().getResource("/application.css").toExternalForm();
        stage.setResizable(false);
        System.out.println(cssPath);
        scene.getStylesheets().add(cssPath);
        stage.setScene(scene);
        stage.setTitle("Video transcoder");
        stage.show();


    }
}

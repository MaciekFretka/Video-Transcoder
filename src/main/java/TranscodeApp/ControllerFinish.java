package TranscodeApp;

import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Label;

import java.net.URL;
import java.util.ResourceBundle;

public class ControllerFinish implements Initializable {

    @FXML
    public Label finishLabel;

    @Override
    public void initialize(URL location, ResourceBundle resources) {

    }


    public void setFinishText(String text){
        finishLabel.setText(text);
    }

}

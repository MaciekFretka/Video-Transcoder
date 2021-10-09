package TranscodeApp;

import java.io.IOException;

public interface Observer {

    void updateProgress();
    void finish() throws IOException;

}

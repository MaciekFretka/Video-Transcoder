package TranscodeApp;

//Komenda do wygenerowania nagłówka : javac -h c -d . Transcoder.java
public class Transcoder {

    public synchronized native void transcode(String inputFilePath,String outputFilePath,String transcodingPreset, String crfValue);
    static {
       // System.load("C:\\Users\\mswar\\source\\repos\\Nowy\\bin\\Transcoding.dll");
        System.load("C:\\Users\\mswar\\source\\repos\\TranscodeDLL\\bin\\TranscodeDLL.dll");
    }
}

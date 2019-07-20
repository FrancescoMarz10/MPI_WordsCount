import java.io.*;
import java.util.*;
import java.io.FileWriter;


public class fileGenerator1 {
    private static Random rand = new Random();
    private static Scanner input = new Scanner(System.in);

    public static void main(String[] args) {
        String fileName = "words.txt";
        File wordList = new File(fileName);
        List<String> words = new ArrayList<>();
        
        Scanner reader = null;

        try {
            reader = new Scanner(wordList);
        } catch (FileNotFoundException e) {
            System.out.println("file \"" + fileName + "\" not found");
            System.exit(0);
        }

        while(reader.hasNextLine()) {
            String word = reader.nextLine();
            words.add(word);
        } 

        try{
             createFile(words);
        }
        catch(IOException e){
            System.out.println("errore");
        }
       
       
    }

    public static void createFile(List<String> words) throws IOException{
        int wordNum = words.size();
        String attempt;
        int place;
        ArrayList<String> parole = new ArrayList<>();

        //IL VALORE ALL'INTERNO DEL FOR INDICA IL NUMERO DI PAROLE GENERATE CASUALMENTE NEL FILE.
        for(int i = 0; i <= 1000000; i++) {
            place = rand.nextInt(wordNum);
            String s = words.get(place);
            parole.add(s);
        }
        FileWriter writer = new FileWriter("fileGenerato.txt"); 
        for(String str: parole) {
          writer.write(str+" ");
        }
        writer.close();
    }
}

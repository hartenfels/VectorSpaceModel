import java.io.*;


public class InvertedIndex implements Serializable
{

    public void
    addToken(int id, String token)
    {
        throw new RuntimeException("Not implemented.");
    }


    public void
    stash(String path)
    {
        try
        {
            FileOutputStream   file = new   FileOutputStream(path);
            ObjectOutputStream objs = new ObjectOutputStream(file);
            objs.writeObject(this);
            objs.close();
            file.close();
        }
        catch (IOException e)
        {
            System.err.println("Error stashing `%s`: %s".format(path, e.getMessage()));
        }
    }


    public static InvertedIndex
    unstash(String path)
    {
        InvertedIndex index = null;

        try
        {
            FileInputStream   file = new FileInputStream(path);
            ObjectInputStream objs = new ObjectInputStream(file);
            index = (InvertedIndex) objs.readObject();
            objs.close();
            file.close();
        }
        catch (IOException | ClassNotFoundException e)
        {
            System.err.println("Error unstashing `%s`: %s".format(path, e.getMessage()));
        }

        return index;
    }

}

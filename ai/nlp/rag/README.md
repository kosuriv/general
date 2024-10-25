# What it Does 

It contains code for NLP  Question Answering application using RAG method. It uses following

LM Model = "EleutherAI/gpt-neo-2.7B" 

Emebeddings model = "sentence-transformers/all-MiniLM-L6-v2"

vector data base = FAISS 



## Limitations 

1. Transformer generates only 100 words at the Max

2. It considers only one document 

3. Currently stored index is not used , everytime it needs to run 

4. It creates UI as web server using Flask python package 

5. Evaluation is not done 
'


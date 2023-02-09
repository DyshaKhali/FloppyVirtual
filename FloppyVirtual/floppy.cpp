#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtGui/QPalette>

#include <iostream>
#include <string>
#include <algorithm>
#include <list>

struct FloppyFile
{
public:
	FloppyFile(std::string label,size_t size,size_t startAddress) :
	m_label(label),
	m_size(size),
	m_startAddress(startAddress)
	{

	}

	inline std::string getLabel()
	{
		return m_label;
	}

	inline size_t getSize()
	{
		return m_size;
	}

	inline size_t getAddress()
	{
		return m_startAddress;
	}

	
	friend std::ostream& operator<<(std::ostream& os,const FloppyFile& file)
	{
		os << "File: "         << file.m_label			     << std::endl;
		os << "Size: "         << file.m_size  << " byte(s)" << std::endl;
		os << "Disk address: " << file.m_startAddress        << std::endl;
		
		return os;
	}	

private:
	std::string m_label = nullptr;
	size_t      m_size = 0;
	size_t      m_startAddress = 0;
};

class Floppy
{
public:
	Floppy()
	{
		
	}

	bool writeFile(std::string label,size_t size)
	{
		//Search for equal strings
		auto sameIt = std::find_if
		(
			std::begin(m_files),std::end(m_files),
			[label](FloppyFile file)
			{
				std::string newFileName(label);
				std::string fileName(file.getLabel());
				
				//Convert string to lower case 
				std::transform(std::begin(newFileName),std::end(newFileName),std::begin(newFileName),[](unsigned char c){return std::tolower(c);});
				std::transform(std::begin(fileName)   ,std::end(fileName)   ,std::begin(fileName)   ,[](unsigned char c){return std::tolower(c);});

				return newFileName == fileName;
			}
		);

		if(sameIt != std::end(m_files))
		{
			std::cout << "|Failed Create File|" << std::endl;
			std::cout << "File with this name already exists" << std::endl;
			return false;
		}


		//                       32 KB
		if(size >= 18 && size <= 32*1024)
		{			
			size_t address = 0;
			if(m_files.empty())
			{
				address = 0;
			}
			else
			{
				for(auto it = std::begin(m_files);it != std::end(m_files);++it)
				{
					if(std::next(it) != std::end(m_files))
					{
						if(std::next(it)->getAddress() - (it->getAddress()+it->getSize()) >= size)
						{
							address = it->getAddress() + it->getSize();

							break;
						}
					}
					else
					{
						if(m_files.size() == 1 && it->getAddress() >= size)
							address = 0;
						else
							address = it->getAddress() + it->getSize();
					}
				}
			}
				
			if(address + size > m_maxSize)
			{
				std::cout << "|Failed Create File|"     << std::endl;
				std::cout << "No free memory for file!" << std::endl;
				return false;
			}
			
			FloppyFile file(label,size,address);
			m_files.push_back(file);
			
			std::cout << "|File Created|" << std::endl;
			std::cout << file << std::endl;

			m_files.sort([](FloppyFile a,FloppyFile b){return a.getAddress() < b.getAddress();});
			
			return true;
		}
		
		std::cout << "|Failed Create File|"       << std::endl;
		std::cout << "File too small or too big." << std::endl << std::endl;
		return false;
	}
	
	bool deleteFile(std::string label)
	{	
		auto fileIt = std::find_if(std::begin(m_files),std::end(m_files),[label](FloppyFile& file){return file.getLabel() == label;});
		
		if(fileIt != std::end(m_files))
		{
			std::cout << "|File Deleted|" << std::endl;
			std::cout << *fileIt    << std::endl;
			
			m_files.erase(fileIt);
			
			return true;
		}
		
		std::cout << "|Failed delete file|"       << std::endl;
		std::cout << label << " - No such files!" << std::endl;
		return false;
	}
	
	void status()
	{
		std::cout << "|Floppy Status|" << std::endl;
		for(auto&& file : m_files)
			std::cout << file << std::endl;
	}
private:
	std::list<FloppyFile> m_files;
	const size_t m_maxSize = 360 * 1024;//360 KB
};

class FloppyVirtualWindow : public QWidget
{
public:
	FloppyVirtualWindow(QWidget* parent = nullptr) :
	QWidget(parent)
	{
		this->setWindowTitle("Floppy Virtual");
		this->setGeometry(QRect(0,0,640,300));
		this->setFixedSize(640,300);
		
		m_logLabel = new QLabel(this);
		m_logLabel->setText("");
		m_logLabel->setGeometry(QRect(250,25,200,25));
		m_logLabel->show();

		m_createFloppyDiskButton = new QPushButton(this);
		m_createFloppyDiskButton->setText("Create Virtual Floppy Disk");
		m_createFloppyDiskButton->setGeometry(QRect(25,25,200,25));
		QPushButton::connect(m_createFloppyDiskButton,&QPushButton::clicked,
		[this]()
		{
			if(!this->m_floppyDisk)	
			{
				this->m_logLabel->setText("Floppy Disk Created");

				this->m_createFloppyDiskButton->hide();
				this->m_floppyDisk = new Floppy();
				
				this->m_writeFileButton->show();
				this->m_writeFileLineEdit->show();
				this->m_writeFileSizeLineEdit->show();

				this->m_deleteFileButton->show();
				this->m_deleteFileLineEdit->show();

				this->m_statusButton->show();
			}
		}
		);

		m_writeFileButton = new QPushButton(this);
		m_writeFileButton->setGeometry(25,75,200,25);
		m_writeFileButton->setText("Write File");
		QPushButton::connect(m_writeFileButton,&QPushButton::clicked,
		[this]()
		{
			if(this->m_writeFileLineEdit->text().isEmpty())
				this->m_logLabel->setText("FileName is empty");
			else
			{
				this->m_logLabel->setText("File Written");
				this->m_floppyDisk->writeFile(this->m_writeFileLineEdit->text().toStdString(), this->m_writeFileSizeLineEdit->text().toUInt());
			}
		}
		);
		m_writeFileButton->hide();

		m_writeFileLineEdit = new QLineEdit(this);
		m_writeFileLineEdit->setGeometry(240,75,175,25);
		m_writeFileLineEdit->setText("");
		m_writeFileLineEdit->hide();

		m_writeFileSizeLineEdit = new QLineEdit(this);
		m_writeFileSizeLineEdit->setGeometry(430,75,125,25);
		m_writeFileSizeLineEdit->setText("");
		m_writeFileSizeLineEdit->hide();
		


		m_deleteFileButton = new QPushButton(this);
		m_deleteFileButton->setGeometry(25,115,200,25);
		m_deleteFileButton->setText("Delete File");
		QPushButton::connect(m_deleteFileButton,&QPushButton::clicked,
		[this]()
		{
			if(this->m_deleteFileLineEdit->text().isEmpty())
				this->m_logLabel->setText("FileName is empty");
			else
			{
				this->m_logLabel->setText("File deleted");
				this->m_floppyDisk->deleteFile(this->m_deleteFileLineEdit->text().toStdString());
			}
		}
		);
		m_deleteFileButton->hide();

		m_deleteFileLineEdit = new QLineEdit(this);
		m_deleteFileLineEdit->setGeometry(240,115,175,25);
		m_deleteFileLineEdit->setText("");
		m_deleteFileLineEdit->hide();

		m_statusButton = new QPushButton(this);
		m_statusButton->setGeometry(25,155,200,25);
		m_statusButton->setText("Status");
		m_statusButton->hide();
		QPushButton::connect(m_statusButton,&QPushButton::clicked,
		[this]()
		{
			this->m_logLabel->setText("Disk Status");
			this->m_floppyDisk->status();
		}
		);
	}

	~FloppyVirtualWindow()
	{
		if(m_logLabel)
			delete m_logLabel;
		
		if(m_floppyDisk)
			delete m_floppyDisk;

		if(m_createFloppyDiskButton)
			delete m_createFloppyDiskButton;
		if(m_writeFileButton)
			delete m_writeFileButton;
		if(m_writeFileSizeLineEdit)
			delete m_writeFileSizeLineEdit;

		if(m_deleteFileButton)
			delete m_deleteFileButton;
		if(m_deleteFileLineEdit)
			delete m_deleteFileLineEdit;

		if(m_statusButton)
			delete m_statusButton;
	}
private:
	Floppy* m_floppyDisk = nullptr;

	QLabel*      m_logLabel = nullptr;

	QPushButton* m_createFloppyDiskButton = nullptr;

	QPushButton* m_writeFileButton       = nullptr;
	QLineEdit*   m_writeFileLineEdit     = nullptr;
	QLineEdit*   m_writeFileSizeLineEdit = nullptr;

	QPushButton* m_deleteFileButton   = nullptr;
	QLineEdit*   m_deleteFileLineEdit = nullptr;

	QPushButton* m_statusButton = nullptr;
};

int main(int argc,char* argv[])
{	
	QApplication app(argc,argv);
	FloppyVirtualWindow window;
	
	window.show();
	return app.exec();
}
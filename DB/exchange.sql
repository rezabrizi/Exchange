CREATE TABLE Instruments (
ID varchar(20) NOT NULL,
Name varchar(80) NOT NULL,
Type varchar(20) NOT NULL,
ListingDate BIGINT NOT NULL,
PRIMARY KEY (ID)
);

CREATE TABLE Clients (
ID varchar(20) NOT NULL,
Name varchar(80) NOT NULL,
TCPIPAddress varchar(45) NOT NULL,  -- IPv6 addresses can be up to 45 characters long
Passkey varchar(45) NOT NULL,
PRIMARY KEY (ID)
);

CREATE TABLE Orders (
OrderKey varchar(50) NOT NULL,  -- Adjust based on expected key size
OrderID INT NOT NULL,
InstrumentID varchar(20) NOT NULL,  -- Same as Instruments.ID size
ClientID varchar(20) NOT NULL,  -- Same as Clients.ID size
Price FLOAT NOT NULL,
Quantity INT NOT NULL,
BidOrAsk boolean NOT NULL,
Type varchar(50) NOT NULL,  -- Adjust based on expected type size
EntryTime BIGINT NOT NULL,
CancelTime BIGINT,
PRIMARY KEY (OrderKey),
FOREIGN KEY (InstrumentID) REFERENCES Instruments(ID),
FOREIGN KEY (ClientID) REFERENCES Clients(ID)
);

CREATE TABLE Executions (
ExecutionID INT NOT NULL,
OrderKey varchar(50) NOT NULL,  -- Same as Orders.OrderKey size
ExecutionTime BIGINT NOT NULL,
Price FLOAT NOT NULL,
Quantity INT NOT NULL,
PRIMARY KEY (ExecutionID, OrderKey),
FOREIGN KEY (OrderKey) REFERENCES Orders(OrderKey)
);

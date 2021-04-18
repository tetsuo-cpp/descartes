const
   firstName = 'alex';
   lastName  = 'cameron';
   age       =  100;

type
   AgeType    = integer;
   PersonType = record
                   fn  : string;
                   ln  : string;
                   age : AgeType;
                end;
   PersonPtr  =  ^PersonType;

var
   alex :  PersonType;

procedure printPerson(p : PersonPtr);
begin
   writeln(p^.fn);
   writeln(p^.ln);
   writeln(p^.age);
   p^.age := p^.age + 1;
end;

begin
   alex.fn := firstName;
   alex.ln := lastName;
   alex.age := age;
   printPerson(alex);
   printPerson(alex);
end.
